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
		var repartition = $('input[name="gpt_recreate"]');
		var repartition_span = $('input[name="gpt_recreate"]');
		var bootmenu = $('input[name="bootmenu_recreate"]');
		var recovery = $('input[name="bootmenu_recovery"]');
		var check_recovery = function() {
			if (bootmenu.prop("checked")) {
				recovery.removeAttr("disabled");
			} else {
				recovery.prop("checked", false);
				recovery.attr("disabled", "disabled");
			}
		}
		var check_bootmenu = function() {
			if (repartition.prop("checked")) {
				bootmenu.prop("checked", true);
				bootmenu.attr("disabled", "disabled");
			} else {
				bootmenu.prop("checked", false);
				bootmenu.attr("disabled", "disabled");
				//bootmenu.removeAttr("disabled");
			}
			check_recovery();
		};

		check_bootmenu();
		check_recovery();
		repartition.change(
			function(){
				check_bootmenu();
			}
		);
		bootmenu.change(
			function(){
				check_recovery();
			}
		);
	});
	</script>
	<% if [ -f /data/.recovery/settings.tar.gz ]; then %>
	<input type="hidden" name="backup_available">
	<% else %>
	<input type="hidden" name="backup_available" disabled>
	<% fi %>
    <div class="field">
		<div class="control">
			<label class="checkbox">
				<input type="checkbox" name="backup_settings" checked>
				Backup current settings before flashing</label>
			</label>
		</div>
	</div>
	<div class="field">
		<div class="control">
			<label class="checkbox">
				<input type="checkbox" name="restore_settings" checked>
				Restore settings after flashing
			</label>
		</div>
	</div>
	<div class="field">
		<div class="control">
			<label class="checkbox">
				<input type="checkbox" name="password_remove">
				Set empty root password
			</label>
		</div>
	</div>
<% if is_gpt_uboot; then %>
	<div class="field">
		<div class="control">
			<label class="checkbox">
				<input type="checkbox" name="gpt_recreate">
				Re-Create partition table (GPT)
				<span class="tag is-danger">All data will be lost (including the dreambox-data parition)!</span>
			</label>
		</div>
	</div>
	<div class="field">
		<div class="control">
			<label class="checkbox">
				<input type="checkbox" name="bootmenu_recreate">
				Re-Create bootmenu
				<span class="tag is-danger">All existing bootmenu entries will be lost!</span>
			</label>
		</div>
	</div>
	<div class="field ml-5 is-hidden">
		<div class="control">
			<label class="checkbox">
				<input type="checkbox" name="bootmenu_recovery">
				Add Recovery system to bootmenu
				<span class="tag is-info">Allows to start the recovery using the Remote Control</span>
			</label>
		</div>
	</div>
<% fi %>
