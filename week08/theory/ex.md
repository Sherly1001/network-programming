## 1. so sánh việc sử dụng select() và fork()
- dùng select() sẽ không cần phải tạo child process cho mỗi socket như fork()
- nhưng select() sẽ khó sử dụng hơn fork()

## 2. Mô tả chi tiêt lại I/O multiplexing
- kernel sẽ theo dõi nhiều descriptor file cùng một lúc
- process sẽ bị block cho đến khi kernel báo lại khi có descriptor file đã sẵn sàng hoặc hết thời gian chờ timeout
- sau khi descriptor file sẵn sàng thì process có thể gọi các system call read, write với file và block cho đến khi xử lý xong
