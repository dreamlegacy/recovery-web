try_mount_data()
{
	local device="/dev/mmcblk0p3"
	[ -b ${device} ] && (mountpoint -q /data || (mkdir -p /data && mount -o ro ${device} /data))
}
