#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcgiapp.h>

struct fcgi_context {
	FCGX_Stream *in;
	FCGX_Stream *out;
	FCGX_Stream *err;
	FCGX_ParamArray envp;
};

enum fcgi_status {
	HTTP_BAD_REQUEST = 400,
	HTTP_FORBIDDEN = 403,
	HTTP_NOT_FOUND = 404,
	HTTP_INTERNAL_SERVER_ERROR = 500,
};

static char *urldecode(char *src)
{
	char *res = src;
	char *dst = src;
	int esc = 0;
	int val = 0;

	while (*src) {
		int s = *src++;
		if (esc == 0) {
			if (s == '%')
				esc++;
			else if (s == '+')
				*dst++ = ' ';
			else
				*dst++ = s;
		} else if (isxdigit(s)) {
			if (s >= 'a')
				s -= 'a' - 'A';
			else if (s >= 'A')
				s -= 'A' - 10;
			else
				s -= '0';
			val <<= 4;
			val |= s;
			if (esc++ == 2) {
				*dst++ = val & 0xff;
				esc = 0;
			}
		}
	}

	*dst = '\0';
	return esc == 0 ? res : NULL;
}

static inline int fcgi_accept(struct fcgi_context *ctx)
{
	return FCGX_Accept(&ctx->in, &ctx->out, &ctx->err, &ctx->envp);
}

static inline void fcgi_finish(struct fcgi_context *ctx)
{
	FCGX_Finish();
}

static inline const char *fcgi_getenv(struct fcgi_context *ctx, const char *name)
{
	return FCGX_GetParam(name, ctx->envp);
}

static void fcgi_vprintf(struct fcgi_context *ctx, const char *fmt, va_list ap_out)
{
    #if !defined(NDEBUG)
	va_list ap_err;

	va_copy(ap_err, ap_out);
	FCGX_VFPrintF(ctx->err, fmt, ap_err);
    #endif
	FCGX_VFPrintF(ctx->out, fmt, ap_out);
}

static void __attribute__ ((__format__ (__printf__, (2), (3)))) fcgi_printf(struct fcgi_context *ctx, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fcgi_vprintf(ctx, fmt, ap);
	va_end(ap);
}

static inline void fcgi_newline(struct fcgi_context *ctx)
{
	fcgi_printf(ctx, "\r\n");
}

static void __attribute__ ((__format__ (__printf__, (3), (4)))) fcgi_header(struct fcgi_context *ctx, const char *key, const char *fmt, ...)
{
	va_list ap;

	fcgi_printf(ctx, "%s: ", key);

	va_start(ap, fmt);
	fcgi_vprintf(ctx, fmt, ap);
	va_end(ap);

	fcgi_newline(ctx);
}

static bool fcgi_status(struct fcgi_context *ctx, enum fcgi_status code)
{
	const char *msg = NULL;

	switch (code) {
	case HTTP_BAD_REQUEST:
		msg = "Bad Request";
		break;
	case HTTP_FORBIDDEN:
		msg = "Forbidden";
		break;
	case HTTP_NOT_FOUND:
		msg = "Not Found";
		break;
	case HTTP_INTERNAL_SERVER_ERROR:
		msg = "Internal Server Error";
		break;
	}

	fcgi_header(ctx, "Status", "%u %s", code, msg);
	fcgi_newline(ctx);
	return true;
}

static bool sendfile_filename(struct fcgi_context *ctx, const char *filename)
{
	const char *base;
	struct stat st;

	if (filename[0] != '/')
		return fcgi_status(ctx, HTTP_BAD_REQUEST);

	if (stat(filename, &st) < 0) {
		if (errno == EACCES)
			return fcgi_status(ctx, HTTP_FORBIDDEN);
		if (errno == ENOENT)
			return fcgi_status(ctx, HTTP_NOT_FOUND);

		return fcgi_status(ctx, HTTP_INTERNAL_SERVER_ERROR);
	}

	if (access(filename, R_OK) < 0)
		return fcgi_status(ctx, HTTP_FORBIDDEN);

	base = basename(filename);
	fcgi_header(ctx, "Content-type", "application/octet-stream");
	fcgi_header(ctx, "Content-Disposition", "attachment;filename=\"%s%s\"", base, S_ISBLK(st.st_mode) ? ".img" : "");
	fcgi_header(ctx, "X-SendFile", filename);
	fcgi_newline(ctx);
	return true;
}

static bool sendfile_field(struct fcgi_context *ctx, const char *key, const char *val)
{
	if (!strcmp(key, "filename") && *val != '\0')
		return sendfile_filename(ctx, val);

	return false;
}

static bool sendfile_query_string(struct fcgi_context *ctx, char *qstr)
{
	char *t;

	for (t = qstr; *t; t++)
		if (*t == '+')
			*t = ' ';

	for (t = strtok(qstr, "&;"); t != NULL; t = strtok(NULL, "&;")) {
		char *key = urldecode(t);
		if (key == NULL)
			break;

		char *val = strchr(key, '=');
		if (val == NULL)
			break;

		*val++ = '\0';
		if (sendfile_field(ctx, key, val))
			return true;
	}

	return false;
}

#if !defined(NDEBUG)
static bool debug_query_string(struct fcgi_context *ctx, char *qstr)
{
	char **env = ctx->envp;
	char *t;

	fcgi_header(ctx, "Content-type", "text/plain");
	fcgi_newline(ctx);
	fcgi_printf(ctx, "encoded=\"%s\"", qstr);
	fcgi_newline(ctx);
	fcgi_newline(ctx);

	while (*env) {
		fcgi_printf(ctx, "%s", *env);
		fcgi_newline(ctx);
		env++;
	}

	for (t = qstr; *t; t++)
		if (*t == '+')
			*t = ' ';

	for (t = strtok(qstr, "&;"); t != NULL; t = strtok(NULL, "&;")) {
		char *key = urldecode(t);
		if (key) {
			char *val = strchr(key, '=');
			if (val)
				*val++ = '\0';
			else
				val = "";
			fcgi_printf(ctx, "key='%s' val='%s'", key, val);
			fcgi_newline(ctx);
		}
	}

	return true;
}
#endif

static bool sendfile_request(struct fcgi_context *ctx)
{
	const char *qstr = fcgi_getenv(ctx, "QUERY_STRING");
	char *dup;
	bool ret;

	if (qstr == NULL || *qstr == '\0') {
		const char *request_uri = fcgi_getenv(ctx, "REQUEST_URI");
		if (request_uri == NULL)
			return false;
		qstr = strchr(request_uri, '?');
		if (qstr == NULL)
			return false;
		qstr++;
		if (*qstr == '\0')
			return false;
	}

	dup = strdup(qstr);
	if (dup == NULL)
		return false;

	ret = sendfile_query_string(ctx, dup);
    #if !defined(NDEBUG)
	if (ret == false) {
		strcpy(dup, qstr);
		ret = debug_query_string(ctx, dup);
	}
    #endif

	free(dup);
	return ret;
}

static bool fcgi_request(struct fcgi_context *ctx)
{
	const char *name;

	name = fcgi_getenv(ctx, "SCRIPT_NAME");
	if (name == NULL)
		return false;

	if (!strcmp(name, "/sendfile/"))
		return sendfile_request(ctx);

	return false;
}

int main(int argc, char *argv[])
{
	struct fcgi_context ctx;

	while (fcgi_accept(&ctx) >= 0) {
		if (!fcgi_request(&ctx))
			fcgi_status(&ctx, HTTP_BAD_REQUEST);
		fcgi_finish(&ctx);
	}

	return 0;
}
