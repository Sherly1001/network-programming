## 1. Trình bày sự khác nhau giữa recvfrom() và recv(); sendto() và send()
- `recvfrom` là nhận từ một socket address được chỉ định sẵn trong param của `recvfrom()`, được dùng trong udp
- `recv` là nhận từ socket đã được kết nối trước đó, được dùng trong tcp
- `sendto` là gửi đến một socket address được chỉ định sẵn trong param của `sendto()`, được dùng trong udp
- `send` là gửi đến socket đã được kết nối trước đó, được dùng trong tcp

## 2. Trình bày rõ hơn ý nghĩa và chi tiết từng cờ trong lệnh
- `recv()`
  - MSG_CONNTERM: recv chỉ chạy khi bị ngắt kết nối
  - MSG_OOB: nhận data out of band
  - MSG_PEEK: đọc data trong socket mà không xóa
  - MSG_WAITALL: đọc và đợi đến khi đọc đủ số lượng data được gửi đến
- `send()`
  - MSG_OOB: gửi data out of band
  - MSG_DONTROUTE: không chuyển dữ liệu tới gateway, chỉ gửi tới các máy trong cùng mạng

## 3. Miêu tả rõ hơn slide 18 (process connections trong TCP)
- khi server gọi `listen()`, thì server sẫn sàng cho việc đợi các kết nối từ client
- client gọi `connect()` sẽ gửi tín hiệu SYN đến server
- server nhận được tín hiệu SYN từ client, server sẽ gửi lại tín hiệu SYN/ACK cho client
- client nhận được SYN/ACK từ server sẽ tiếp tục gửi tín hiệu ACK đến server
- server nhận được ACK thì kết nối được thiết lập và socket connection được đưa vào một queue
- mỗi khi server gọi `accept()` thì nó sẽ pop một connection từ queue ra và sử lý
