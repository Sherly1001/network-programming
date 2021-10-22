## 1. Trình bày rõ hơn ý nghĩa và chi tiết từng cờ trong lệnh
### recvfrom()
- MSG_PEEK: đọc dữ liệu trong socket buffer, nhưng không xóa sau khi đọc xong
- MSG_OOB: nhận dữ liệu out of band
- MSG_WAITALL: chờ tới khi nhận đủ dữ liệu, sẽ bị block nếu sảy ra:
  - đứt kết nối
  - cờ MSG_PEEK được bật
  - có lỗi đang chờ trong socket
  - một signal được emit

### sendto()
- MSG_OOB: gửi dữ liệu out of band
- MSG_DONTROUTE: không chuyển dữ liệu tới gateway, chỉ gửi tới các máy trong cùng mạng

## 2. Giá trị trả về EMSGSIZE trong câu lệnh sendto() có ý nghĩa gì?
- khi mà tầng ứng dụng gửi một datagram lớn hơn buffer của socket, hàm sendto() sẽ trả về lỗi EMSGSIZE để tầng ứng dụng có thể xử lý lỗi và khắc phục như gửi lại một datagram nhỏ hơn

## 3. Các trường hợp nào nên sử dụng UDP và tại sao?
- sử dụng khi tốc độ truyền được ưu tiên, và việc sửa lỗi là không cần thiết
- ví dụ như sử dụng giao thức UDP trong live stream video hoặc trong các game online
- vì việc live stream hay game online cần tính thời gian thực, nếu trong trường hợp client không nhận được một gói tin UDP thì video hoặc âm thanh chỉ bị méo một chút, sau khi tiếp tục nhận tiếp các gói tin thì video sẽ tiếp tục phát bình thường,
- hay trong game online, sau khi nhận gói tin udp mới thì vị trị người chơi sẽ lại được cập nhật trên bản đồ
