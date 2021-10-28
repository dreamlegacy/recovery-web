try_mount_data()
{
	local device="/dev/dreambox-data"
	if [ ! -b "${device}" ]; then
		device="/dev/disk/by-partlabel/dreambox-data"
	fi
	[ -b ${device} ] && (mountpoint -q /data || (mkdir -p /data && mount -o ro ${device} /data))
}

is_gpt()
{
	PARTITION_TABLE=`parted /dev/mmcblk0 print | grep Partition | awk '{print $3}'`
	[ $PARTITION_TABLE == "gpt" ]
}

is_gpt_uboot()
{
	case "$product" in
		ONE|TWO|SEVEN)
			;;
		*)
			false
			return
			;;
	esac

	local bl_hash=$(dd "if=/dev/bootloader" "bs=$((${size} + 512))" count=1 2>/dev/null | dd skip=1 2>/dev/null | sha256sum | awk '{print $1}')
	case "$bl_hash" in
			1938acdc29f2639f9b8bafc13ecb43c3d0c065ee3768b64b285d7d911ce1597d|c1c8f9a48b573198bc498698bc9c44eef5d0394bb84bb37ef59195853d012b1f|430990893f1de7984a017767ab6bc4a9086ed429a45d15930ace1773914c7fda)
					false
					return
					;;
	esac
	true
}
