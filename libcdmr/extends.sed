s/NCSTREAM_H/NCSTREAMX_H/
/[#]define[ ]*NCSTREAMX_H/a\
\
enum Sort {\
_Null		= 0,\
_Attribute     	= 1,\
_Dimension     	= 2,\
_Variable      	= 3,\
_Structure     	= 4,\
_EnumTypedef   	= 5,\
_EnumType      	= 6,\
_Group		= 7,\
_Header		= 8,\
_Data		= 9,\
_Range		= 10,\
_Section       	= 11,\
_StructureData	= 12,\
_Error		= 13\
} Sort;\

/^message[ ][ ]*[a-zA-Z0-9_$]*/a\
    option extends="Notes notes";
