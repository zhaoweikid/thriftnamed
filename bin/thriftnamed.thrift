namespace cpp qfpay

service Named {
    void ping(),
	string query(1:string name),
	i32 report(1:string name, 2:i32 ts, 3:i64 n),
	i32 sync(1:string name, 2:string ip, 3:i32 ts, 4:i64 n),
}
