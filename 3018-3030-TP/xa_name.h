#ifndef XA_NAME
#define XA_NAME

int xa_get_ticketName(int ticketType, unsigned char *chName, unsigned char *enName);

//line:0x110000+line(hex)
int xa_get_lineName(int line, unsigned char *chName, unsigned char *enName);

//staion :0x0900+line_stationid(hex)
int xa_get_stationName(int station, unsigned char *chName, unsigned char *enName);

void xa_get_rejectName(int reject, unsigned char *chName, unsigned char *enName);
//line:0x110000+line(hex)
//public static native int get_all_line_name(int language, byte* pernumber, byte* pername );
int xa_get_all_lineName(int language, unsigned char *perNumber, unsigned char *lineName);
//首先获取线路数，同时记录线路指针
//返回0表示无线路数，其它数表示线路数目
int xa_get_line_number();
//根据线路代码逐次获取线路名称，同时记录所属车站数
//返回0表示成功
//其它值失败
int xa_get_line_index_name(int lineIndex, char *chName, char *enName, int *LocationID);
//根据线路索引获取指定线路的所属车站数
//返回0表示无线路下属车站数，其它数表示所属线路的车站数目
int xa_get_line_index_station_number(int lineIndex);

//根据线路代码逐次获取线路车站名称，同时记录所属车站数
//返回0表示成功
//其它值失败
int xa_get_line_index_station_index_name(int lineIndex, int stationIndex, char *chName, char *enName, int *LocationID);

#endif