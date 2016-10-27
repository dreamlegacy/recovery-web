try_mount_data()
{
	local device="/dev/disk/by-label/dreambox-data"
	[ -b ${device} ] && (mountpoint -q /data || (mkdir -p /data && mount -o ro ${device} /data))
}
