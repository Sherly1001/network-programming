## 1.
### So sánh chi tiết lại điểm mạnh và điểm yếu giữa fork() và pthread_create()
fork() | pthread_create()
-- | --
dễ dàng sử dụng | phức tạp hơn một chút
chạy trên các process khác nhau | trên cùng một process
copy lại toàn bộ memory của parent process | shared memory
tạo một process tón tài nguyên hơn | tạo một thread ít tốn tài nguyên

### Nêu lên các trường hợp ứng dụng nên sử dụng fork() và pthread_create()
fork() | pthread_create()
-- | --
không càn giao tiếp giữa các tiến trình | việc trao đổi dữ liệu giữa các tiến trình là cần thiết

## 2. Trình bày chi tiết Semaphore, và Mutex
### Semaphore
- sử dụng biến `count` để giám sát tài nguyên và thread được truy cập tài nguyên đó
- `count` thể hiện số lượng thread tối đa được truy cập vào một tài nguyên
- `count > 0` => semaphore ở trạng thái có thể sử dụng được
- `count = 0` => semaphore bị khóa, nếu có thread gọi hàm `down()` thì thread đó sẽ được đua vào `wait_list`
- nếu vẫn còn thread trong `wait_list` thì khi `count` lớn hơn 0 CPU sẽ thực thi thread trong `wait_list`

### Mutex
- cũng giống như Semaphore nhưng chỉ có 2 trạng thái `lock` và `unlock`
- khi ở trạng thái `lock` thì các thread khác sẽ không được truy cập vào tài nguyên và phải chờ cho đến khi `unlock`
