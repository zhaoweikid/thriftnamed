namespace cpp qfpay

struct Service {
	1: required string	ip;
	2: required i16		port;	
	3: required i32		timeout;
}

struct NameInfo {
	1: required string			name;
	2: required list<Service>	services;
}

service Named {
    void ping(),
	NameInfo query(1:string name),
	i32 report(1:string name, 2:i32 timestamp, 3:i64 n),
	i32 sync(1:string auth, 2:string ip, 3:string name, 4:i32 timestamp, 5:i64 n),
}
