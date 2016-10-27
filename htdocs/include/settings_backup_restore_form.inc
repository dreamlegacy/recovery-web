<% source include/functions.inc %>
<% if try_mount_data; then %>
    <div class="action-container"><label class="card"><input type="checkbox" name="backup_settings" checked>Backup current settings before flashing</input></label></div>
    <div class="action-container"><label class="card"><input type="checkbox" name="restore_settings" checked>Restore settings after flashing</input></label></div>
<% fi %>
