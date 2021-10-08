## 1. Trình bày cơ chế NAT, so sánh dynamic NAT và static NAT kèm ứng dụng cụ thể của từng loại
- NAT Là một cơ chế cho phép ánh xạ từ một địa chỉ IP này thành một địa chỉ IP khác. Thường sử dụng trong mạng LAN cần truy cập đến mạng Internet bên ngoài.
- Static NAT:
  - một địa chỉ IP cố định trong LAN sẽ được ánh xạ ra một địa chỉ IP Public cố định trước khi gói tin đi ra Internet
  - phương pháp này không nhằm tiết kiệm địa chỉ IP mà chỉ có mục đích ánh xạ một IP trong LAN ra một IP Public để ẩn IP nguồn trước khi đi ra Internet làm giảm nguy cơ bị tấn công trên mạng
- Dynamic NAT:
  - là một giải pháp tiết kiệm IP Public cho NAT tĩnh
  - cho phép ánh xạ cả dải IP trong LAN ra một dải IP Public cố định ra bên ngoài

## 2. So sánh địa chỉ Ipv4 và Ipv6 và ý nghĩa sử dụng địa chỉ Ipv6
Ipv4 | Ipv6
--- | ---
gồm 32 bits | gồm 128 bits
chia thành các lớp mạng A, B, C,.. | sử dụng subnet để điểu chỉnh kích thước mạng
sử dụng không gian địa chỉ loại lớp để sử dụng phát đa hướng | sử dụng một không gian địa chỉ tích hợp cho phát đa hướng
sử dụng các địa chỉ phát sóng truyền hình trực tuyến bắt buộc mỗi thiết bị dừng và xem xét các gói | sử dụng các nhóm phát đa hướng
sử dụng 0.0.0.0 làm địa chỉ không xác định và địa chỉ loại lớp (127.0.0.1) cho loopback | sử dụng :: và :: 1 làm địa chỉ không xác định và loopback tương ứng
sử dụng các địa chỉ công cộng duy nhất trên toàn cầu cho lưu lượng truy cập và các địa chỉ riêng tư của trực tuyến | sử dụng địa chỉ unicast duy nhất trên toàn cầu và địa chỉ cục bộ (FD00 :: / 8)

- ý nghĩa sử dụng Ipv6
  - định tuyến hiệu quả hơn mà không cần phân mảnh gói
  - chất lượng dịch vụ tích hợp (QoS) phân biệt các gói nhạy cảm trễ
  - loại bỏ NAT để mở rộng không gian địa chỉ từ 32 đến 128 bit
  - bảo mật lớp mạng tích hợp (IPsec)
  - cấu hình tự động địa chỉ không trạng thái để quản trị mạng dễ dàng hơn
  - cấu trúc tiêu đề được cải thiện với chi phí xử lý ít hơn

## 3. Trình bày sự giống và khác nhau giữa mô hình OSI và TCP/IP
- giống nhau:
  - đều có kiến trúc phân lớp
  - đều có lớp Network và lớp Transport
  - cùng sử dụng kỹ thuật chuyển Packet

OSI | TCP/IP
--- | ---
có 7 tầng | có 4 tầng
được cho là mô hình cũ, ít dùng | mô hình mới, được chuẩn hóa và sử dụng phổ biến
tiếp cận theo chiều dọc | tiếp cận theo chiều ngang
mỗi tầng thực hiện một nhiệm vụ khác nhau, không có sự kết hợp giữa bất cứ tầng nào | trong tầng ứng dụng có tầng trình diễn và tầng phiên được kết hợp với nhau
phát triển mô hình trước sau đó sẽ phát triển giao thức | các giao thức được thiết kế trước sau đó phát triển mô hình
hỗ trợ cả kết nối định tuyến và không dây | hỗ trợ truyền thông không kết nối từ tầng mạng
giao thức độc lập | phụ thuộc vào giao thức
