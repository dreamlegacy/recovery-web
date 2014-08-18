#define _GNU_SOURCE
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcgiapp.h>

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

static bool handle_field(FCGX_Stream *out, const char *key, const char *val)
{
	if (!strcmp(key, "filename") && *val != '\0') {
		struct stat st;
		if (stat(val, &st) == 0) {
			const char *base = basename(val);
			FCGX_FPrintF(out, "Content-type: application/octet-stream\r\n");
			FCGX_FPrintF(out, "Content-Disposition: attachment;filename=\"%s", base);
			if (S_ISBLK(st.st_mode))
				FCGX_FPrintF(out, ".img");
			FCGX_FPrintF(out, "\"\r\n");
			FCGX_FPrintF(out, "X-SendFile: %s\r\n", val);
			FCGX_FPrintF(out, "\r\n");
			return true;
		}
	}

	return false;
}

static bool parse_query_string(FCGX_Stream *out, char *qstr)
{
	char *t;

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
			if (handle_field(out, key, val))
				return true;
		}
	}

	return false;
}

#if !defined(NDEBUG)
static void debug_query_string(FCGX_Stream *out, FCGX_ParamArray envp, char *qstr)
{
	char **env = envp;
	char *t;

	FCGX_FPrintF(out, "Content-type: text/plain\r\n");
	FCGX_FPrintF(out, "\r\n");
	FCGX_FPrintF(out, "encoded=\"%s\"\r\n", qstr);
	FCGX_FPrintF(out, "\r\n");

	while (*env) {
		FCGX_FPrintF(out, "%s\r\n", *env);
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
			FCGX_FPrintF(out, "key='%s' val='%s'\r\n", key, val);
		}
	}
}
#endif

static void handle_sendfile(FCGX_Stream *out, FCGX_Stream *err, FCGX_ParamArray envp)
{
	const char *qstr = FCGX_GetParam("QUERY_STRING", envp);
	char *dup;

	if (qstr == NULL || *qstr == '\0') {
		const char *request_uri = FCGX_GetParam("REQUEST_URI", envp);
		if (request_uri == NULL)
			return;
		qstr = strchr(request_uri, '?');
		if (qstr == NULL)
			return;
		qstr++;
		if (*qstr == '\0')
			return;
	}

	dup = strdup(qstr);
	if (dup == NULL)
		return;

	if (!parse_query_string(out, dup)) {
	#if !defined(NDEBUG)
		strcpy(dup, qstr);
		debug_query_string(out, envp, dup);
	#endif
	}

	free(dup);
}

int main(int argc, char *argv[])
{
	FCGX_Stream *in, *out, *err;
	FCGX_ParamArray envp;

	while (FCGX_Accept(&in, &out, &err, &envp) >= 0) {
		const char *name = FCGX_GetParam("SCRIPT_NAME", envp);
		if (name && !strcmp(name, "/sendfile/"))
			handle_sendfile(out, err, envp);
		FCGX_Finish();
	}

	return 0;
}
