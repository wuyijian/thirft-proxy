struct CheckResultStruct {
    1: i32 code
    2: i32 action
    3: i32 label
    4: i32 level
    5: string detail
}
struct RequestCheckStruct {
    1: i32 appid
    2: i32 gameid
    3: string content
    4: string dataid
    5: string userid
	6: string ip
}
service TisService {
	/*
	 * ping
	 */
	void ping(),
	/**
	 * 重新加载模型
	 */
	i32 reloadModel(),
	/*
	 * 文本检测
	 */
	CheckResultStruct check(1:RequestCheckStruct req)	
}
