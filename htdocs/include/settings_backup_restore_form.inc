<% source include/functions.inc %>
	<script type="text/javascript">
	$(document).ready(function(){
		var backup_settings = $('input[name="backup_settings"]');
		var restore_settings = $('input[name="restore_settings"]');
		var backup_available = $('input[name="backup_available"]');
		var check_restore_settings = function() {
			var is_backup_checked = backup_settings.prop("checked");
			var is_backup_available = !backup_available.is(":disabled");
			var is_restore_enabled = is_backup_available || is_backup_checked;
			if(is_restore_enabled) {
				if(!is_backup_available && is_backup_checked)
					restore_settings.prop("checked", true);
<% if try_mount_data; then %>
				restore_settings.removeAttr("disabled");
<% else %>
				restore_settings.attr("disabled", "disabled");
<% fi %>
			} else {
				restore_settings.prop("checked", false);
				restore_settings.attr("disabled", "disabled");
			}

		};
		check_restore_settings();
		backup_settings.change(
			function(){
			  check_restore_settings();
			}
		);
	});
	</script>
	<% if [ -f /data/.recovery/settings.tar.gz ]; then %>
	<input type="hidden" name="backup_available">
	<% else %>
	<input type="hidden" name="backup_available" disabled>
	<% fi %>
    <div class="action-container"><label class="card"><input type="checkbox" name="backup_settings" checked>Backup current settings before flashing</input></label></div>
    <div class="action-container"><label class="card"><input type="checkbox" name="restore_settings" checked>Restore settings after flashing</input></label></div>
