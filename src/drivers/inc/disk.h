#ifndef __DISK_H__
#define __DISK_H__

#define PORT_DISK0_DATA 0x1f0		 // Data			数据
#define PORT_DISK0_ERR_FEATURE 0x1f1 // Error-Featrues 	错误状态
#define PORT_DISK0_SECTOR_CNT 0x1f2	 // Sector count	操作扇区数
#define PORT_DISK0_SECTOR_LOW 0x1f3	 // LBA low			扇区号/LBA(7:0)
#define PORT_DISK0_SECTOR_MID 0x1f4	 // LBA mid			柱面号(7:0)/LBA(15:8)
#define PORT_DISK0_SECTOR_HIGH 0x1f5 // LBA high		柱面号(15:8)/LBA(23:16)
#define PORT_DISK0_DEVICE 0x1f6		 // Device			设备配置寄存器
#define PORT_DISK0_STATUS_CMD 0x1f7	 // Status			状态控制端口

#define PORT_DISK0_ALT_STA_CTL 0x3f6 // 主控制端口

#define PORT_DISK1_DATA 0x170
#define PORT_DISK1_ERR_FEATURE 0x171
#define PORT_DISK1_SECTOR_CNT 0x172
#define PORT_DISK1_SECTOR_LOW 0x173
#define PORT_DISK1_SECTOR_MID 0x174
#define PORT_DISK1_SECTOR_HIGH 0x175
#define PORT_DISK1_DEVICE 0x176
#define PORT_DISK1_STATUS_CMD 0x177

#define PORT_DISK1_ALT_STA_CTL 0x376

#define DISK_STATUS_BUSY (1 << 7)
#define DISK_STATUS_READY (1 << 6)
#define DISK_STATUS_SEEK (1 << 4)
#define DISK_STATUS_REQ (1 << 3)
#define DISK_STATUS_ERROR (1 << 0)

// ATA/ATAPI-8
struct Disk_Identify_Info
{
	//	0	General configuration bit-significant information
	u16_t General_Config;

	//	1	Obsolete
	u16_t Obsolete0;

	//	2	Specific configuration
	u16_t Specific_Coinfig;

	//	3	Obsolete
	u16_t Obsolete1;

	//	4-5	Retired
	u16_t Retired0[2];

	//	6	Obsolete
	u16_t Obsolete2;

	//	7-8	Reserved for the CompactFlash Association
	u16_t CompactFlash[2];

	//	9	Retired
	u16_t Retired1;

	//	10-19	Serial number (20 ASCII characters)
	u16_t Serial_Number[10];

	//	20-21	Retired
	u16_t Retired2[2];

	//	22	Obsolete
	u16_t Obsolete3;

	//	23-26	Firmware revision(8 ASCII characters)
	u16_t Firmware_Version[4];

	//	27-46	Model number (40 ASCII characters)
	u16_t Model_Number[20];

	//	47	15:8 	80h
	//		7:0  	00h=Reserved
	//			01h-FFh = Maximumnumber of logical sectors that shall be transferred per DRQ data block on READ/WRITE MULTIPLE commands
	u16_t Max_logical_transferred_per_DRQ;

	//	48	Trusted Computing feature set options
	u16_t Trusted_Computing_feature_set_options;

	//	49	Capabilities
	u16_t Capabilities0;

	//	50	Capabilities
	u16_t Capabilities1;

	//	51-52	Obsolete
	u16_t Obsolete4[2];

	//	53	15:8	Free-fall Control Sensitivity
	//		7:3 	Reserved
	//		2 	the fields reported in word 88 are valid
	//		1 	the fields reported in words (70:64) are valid
	u16_t Report_88_70to64_valid;

	//	54-58	Obsolete
	u16_t Obsolete5[5];

	//	59	15:9	Reserved
	//		8	Multiple sector setting is valid
	//		7:0	xxh current setting for number of logical sectors that shall be transferred per DRQ data block on READ/WRITE Multiple commands
	u16_t Mul_Sec_Setting_Valid;

	//	60-61	Total number of user addresssable logical sectors for 28bit CMD
	u16_t Addressable_Logical_Sectors_for_28[2];

	//	62	Obsolete
	u16_t Obsolete6;

	//	63	15:11	Reserved
	//		10:8=1 	Multiword DMA mode 210 is selected
	//		7:3 	Reserved
	//		2:0=1 	Multiword DMA mode 210 and below are supported
	u16_t MultWord_DMA_Select;

	//	64	15:8	Reserved
	//		7:0	PIO mdoes supported
	u16_t PIO_mode_supported;

	//	65	Minimum Multiword DMA transfer cycle time per word
	u16_t Min_MulWord_DMA_cycle_time_per_word;

	//	66	Manufacturer`s recommended Multiword DMA transfer cycle time
	u16_t Manufacture_Recommend_MulWord_DMA_cycle_time;

	//	67	Minimum PIO transfer cycle time without flow control
	u16_t Min_PIO_cycle_time_Flow_Control;

	//	68	Minimum PIO transfer cycle time with IORDY flow control
	u16_t Min_PIO_cycle_time_IOREDY_Flow_Control;

	//	69-70	Reserved
	u16_t Reserved1[2];

	//	71-74	Reserved for the IDENTIFY PACKET DEVICE command
	u16_t Reserved2[4];

	//	75	Queue depth
	u16_t Queue_depth;

	//	76	Serial ATA Capabilities
	u16_t SATA_Capabilities;

	//	77	Reserved for Serial ATA
	u16_t Reserved3;

	//	78	Serial ATA features Supported
	u16_t SATA_features_Supported;

	//	79	Serial ATA features enabled
	u16_t SATA_features_enabled;

	//	80	Major Version number
	u16_t Major_Version;

	//	81	Minor version number
	u16_t Minor_Version;

	//	82	Commands and feature sets supported
	u16_t Cmd_feature_sets_supported0;

	//	83	Commands and feature sets supported
	u16_t Cmd_feature_sets_supported1;

	//	84	Commands and feature sets supported
	u16_t Cmd_feature_sets_supported2;

	//	85	Commands and feature sets supported or enabled
	u16_t Cmd_feature_sets_supported3;

	//	86	Commands and feature sets supported or enabled
	u16_t Cmd_feature_sets_supported4;

	//	87	Commands and feature sets supported or enabled
	u16_t Cmd_feature_sets_supported5;

	//	88	15 	Reserved
	//		14:8=1 	Ultra DMA mode 6543210 is selected
	//		7 	Reserved
	//		6:0=1 	Ultra DMA mode 6543210 and below are suported
	u16_t Ultra_DMA_modes;

	//	89	Time required for Normal Erase mode SECURITY ERASE UNIT command
	u16_t Time_required_Erase_CMD;

	//	90	Time required for an Enhanced Erase mode SECURITY ERASE UNIT command
	u16_t Time_required_Enhanced_CMD;

	//	91	Current APM level value
	u16_t Current_APM_level_Value;

	//	92	Master Password Identifier
	u16_t Master_Password_Identifier;

	//	93	Hardware resset result.The contents of bits (12:0) of this word shall change only during the execution of a hardware reset.
	u16_t HardWare_Reset_Result;

	//	94	Current AAM value
	//		15:8 	Vendor’s recommended AAM value
	//		7:0 	Current AAM value
	u16_t Current_AAM_value;

	//	95	Stream Minimum Request Size
	u16_t Stream_Min_Request_Size;

	//	96	Streaming Transger Time-DMA
	u16_t Streaming_Transger_time_DMA;

	//	97	Streaming Access Latency-DMA and PIO
	u16_t Streaming_Access_Latency_DMA_PIO;

	//	98-99	Streaming Performance Granularity (DWord)
	u16_t Streaming_Performance_Granularity[2];

	//	100-103	Total Number of User Addressable Logical Sectors for 48-bit commands (QWord)
	u16_t Total_user_LBA_for_48_Address_Feature_set[4];

	//	104	Streaming Transger Time-PIO
	u16_t Streaming_Transfer_Time_PIO;

	//	105	Reserved
	u16_t Reserved4;

	//	106	Physical Sector size/Logical Sector Size
	u16_t Physical_Logical_Sector_Size;

	//	107	Inter-seek delay for ISO-7779 acoustic testing in microseconds
	u16_t Inter_seek_delay;

	//	108-111	World wide name
	u16_t World_wide_name[4];

	//	112-115	Reserved
	u16_t Reserved5[4];

	//	116	Reserved for TLC
	u16_t Reserved6;

	//	117-118	Logical sector size (DWord)
	u16_t Words_per_Logical_Sector[2];

	//	119	Commands and feature sets supported (Continued from words 84:82)
	u16_t CMD_feature_Supported;

	//	120	Commands and feature sets supported or enabled (Continued from words 87:85)
	u16_t CMD_feature_Supported_enabled;

	//	121-126	Reserved for expanded supported and enabled settings
	u16_t Reserved7[6];

	//	127	Obsolete
	u16_t Obsolete7;

	//	128	Security status
	u16_t Security_Status;

	//	129-159	Vendor specific
	u16_t Vendor_Specific[31];

	//	160	CFA power mode
	u16_t CFA_Power_mode;

	//	161-167	Reserved for the CompactFlash Association
	u16_t Reserved8[7];

	//	168	Device Nominal Form Factor
	u16_t Dev_from_Factor;

	//	169-175	Reserved
	u16_t Reserved9[7];

	//	176-205	Current media serial number (ATA string)
	u16_t Current_Media_Serial_Number[30];

	//	206	SCT Command Transport
	u16_t SCT_Cmd_Transport;

	//	207-208	Reserved for CE-ATA
	u16_t Reserved10[2];

	//	209	Alignment of logical blocks within a physical block
	u16_t Alignment_Logical_blocks_within_a_physical_block;

	//	210-211	Write-Read-Verify Sector Count Mode 3 (DWord)
	u16_t Write_Read_Verify_Sector_Count_Mode_3[2];

	//	212-213	Write-Read-Verify Sector Count Mode 2 (DWord)
	u16_t Write_Read_Verify_Sector_Count_Mode_2[2];

	//	214	NV Cache Capabilities
	u16_t NV_Cache_Capabilities;

	//	215-216	NV Cache Size in Logical Blocks (DWord)
	u16_t NV_Cache_Size[2];

	//	217	Nominal media rotation rate
	u16_t Nominal_media_rotation_rate;

	//	218	Reserved
	u16_t Reserved11;

	//	219	NV Cache Options
	u16_t NV_Cache_Options;

	//	220	Write-Read-Verify feature set current mode
	u16_t Write_Read_Verify_feature_set_current_mode;

	//	221	Reserved
	u16_t Reserved12;

	//	222	Transport major version number.
	//		0000h or ffffh = device does not report version
	u16_t Transport_Major_Version_Number;

	//	223	Transport Minor version number
	u16_t Transport_Minor_Version_Number;

	//	224-233	Reserved for CE-ATA
	u16_t Reserved13[10];

	//	234	Minimum number of 512-byte data blocks per DOWNLOAD MICROCODE command for mode 03h
	u16_t Mini_blocks_per_CMD;

	//	235	Maximum number of 512-byte data blocks per DOWNLOAD MICROCODE command for mode 03h
	u16_t Max_blocks_per_CMD;

	//	236-254	Reserved
	u16_t Reserved14[19];

	//	255	Integrity word
	//		15:8	Checksum
	//		7:0	Checksum Validity Indicator
	u16_t Integrity_word;
} __attribute__((packed));

#define ATA_READ_CMD 0x24		   // 读命令
#define ATA_WRITE_CMD 0x34		   // 写命令
#define GET_IDENTIFY_DISK_CMD 0xec // 查询参数命令

typedef struct block_buffer_node
{
	u32_t count;												// 请求的扇区数
	u8_t cmd;												// 命令
	u64_t LBA;												// 索引硬盘地址
	u8_t *buffer;											// 指向的缓冲区
	void (*end_handler)(u64_t nr, u64_t parameter); // 命令对应的中断处理程序
	wait_queue_t wait_queue;
}block_buffer_node_t;

typedef struct request_queue
{
	wait_queue_t wait_queue_list;		// 请求硬盘操作的等待队列
	block_buffer_node_t *in_using; // 正在处理的硬盘操作请求
	s64_t block_request_count;			// 剩余请求数
}request_queue_t;

request_queue_t disk_request;
extern block_dev_opt_t IDE_device_operation;

#endif