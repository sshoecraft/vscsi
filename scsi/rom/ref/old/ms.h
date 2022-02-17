
typedef unsigned char UCHAR;
typedef unsigned short UWORD;

typedef struct _INQUIRYDATA {
UCHAR DeviceType : 5;
UCHAR DeviceTypeQualifier : 3;
UCHAR DeviceTypeModifier : 7;
UCHAR RemovableMedia : 1;
#if 0
union {
UCHAR Versions;
#endif
struct {
UCHAR ANSI : 3;
UCHAR ECMA : 3;
UCHAR ISO : 2;
} Version;
//};
UCHAR ResponseDataFormat : 4;
UCHAR HiSupport : 1;
UCHAR NormACA : 1;
UCHAR TerminateTask : 1;
UCHAR AERC : 1;
UCHAR AdditionalLength;
UCHAR Reserved;
UCHAR Addr16 : 1;
UCHAR Addr32 : 1;
UCHAR AckReqQ: 1;
UCHAR MediumChanger : 1;
UCHAR MultiPort : 1;
UCHAR ReservedBit2 : 1;
UCHAR EnclosureServices : 1;
UCHAR ReservedBit3 : 1;
UCHAR SoftReset : 1;
UCHAR CommandQueue : 1;
UCHAR TransferDisable : 1;
UCHAR LinkedCommands : 1;
UCHAR Synchronous : 1;
UCHAR Wide16Bit : 1;
UCHAR Wide32Bit : 1;
UCHAR RelativeAddressing : 1;
UCHAR VendorId[8];
UCHAR ProductId[16];
UCHAR ProductRevisionLevel[4];
UCHAR VendorSpecific[20];
//UCHAR Reserved3[40];
UCHAR IUS : 1;
UCHAR QAS : 1;
UCHAR Clocking : 2;
UCHAR ReservedBit4 : 4;
UCHAR Reserved4;
UWORD Version1;
UWORD Version2;
UWORD Version3;
UWORD Version4;
UWORD Version5;
UWORD Version6;
UWORD Version7;
UWORD Version8;
UCHAR Reserved5[22];
} INQUIRYDATA, *PINQUIRYDATA;
