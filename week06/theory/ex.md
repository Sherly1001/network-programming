## 1. So sánh sự khác nhau giữa Asynchronous I/O Model và signal-driven I/O model
- với signal-drivent I/O model thì kernel sẽ thông báo cho application khi thao tác I/O có thể được bắt đầu
- với async I/O model thì kernel sẽ thông báo cho application khi thao tác I/O hoàn thành xong

## 2. So sánh sự khác nhau giữa fast và slow system call?
- fast system call có thể được hoàn thành mà không bị blocking hoặc bị
waiting. Khi kernel gặp một fast system call, nó sẽ được thực thi ngay lập tức và giữ nguyên tiến trình đã được lên lịch sẵn
- slow system call cần phải đợi task khác hoàn thành để được chạy nên kernel có thể sẽ phải tạm dừng quá trình gọi và chạy một task khác thay thế

## 3. Trình bày rõ hơn về EINTR errors
- EINTR không phải là mội lỗi nguy hiểm
- được trả bởi slow system call khi có một nterrupt signal gửi đến slow system call đó
- application cần viết lại hàm khi gọi slow system call để xử lý trong trường hợp EINTR được trả về
