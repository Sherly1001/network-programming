## 1. trình bày lại khái niệm host address và network address đã nói trên lớp
- ip address gồm 2 phần là network address và host address được chia bởi subnet mask
  - network address dùng để tìm subnet mà máy tính hoặc thiết bị trong subnet đó
  - host address dùng để tìm máy tính hoặc thiết bị trong subnet

## 2. Giải thích rõ hơn khái niệm little-endian và big-endian?
là cách sắp xếp thứ tự các bytes trong bộ nhớ máy tính
- little-endian:
  - byte cuối trong dữ liệu nhị phân sẽ được ghi trước
  - vd: `0x1234abcd` sẽ thành  
    ```
    cd ab 34 12
    ```
- big-endian:
  - byte đầu trong dữ liệu nhị phân sẽ được ghi trước
  - vd: `0x1234abcd` sẽ thành  
    ```
    12 34 ab cd
    ```

## 3. Giải thích rõ hơn lệnh bind(): tại sao TCP client không cần sử dụng?
Vì khi client kết nối đến server thì sẽ gửi cả ip và socket port của client, nên server vẫn sẽ biết được.  
Còn server cần bind cố định port vì nó là server, nếu server có port thay đổi thì client sẽ liên tục phải thay đổi địa chỉ đến server socket port để có thể connect được với server.
