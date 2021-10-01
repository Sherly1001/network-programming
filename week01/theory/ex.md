## 1. So sánh con trỏ hằng và hằng con trỏ, lấy ví dụ cụ thể?
- con trỏ hằng là con trỏ trỏ đến một địa chỉ mà không thể thay đổi giá trị lưu ở địa chỉ mà nó trỏ đến, nhưng việc thay đổi giá trị của con trỏ vẫn hợp lệ.  
  Ex.
  ```c
  int a = 10;
  int b = 11;

  const int *p = &a;
  *p = 100; // Không hợp lệ
  p = &b;   // Hợp lệ
  ```
- hằng con trỏ là con trỏ trỏ đến một địa chỉ cố định, giá trị của con trỏ không được phép thay đổi, nhưng có thể thay đổi giá trị của vùng nhớ mà con trỏ trỏ đến.  
  Ex.
  ```c
  int a = 10;
  int b = 11;

  int* const p = &a;
  *p = 100; // Hợp lệ
  p = &b;   // Không hợp lệ
  ```

## 2. Nêu hiểu biết của em về con trỏ trỏ tới con trỏ?
- con trỏ trỏ tới con trỏ là một con trỏ mà giá trị của nó là địa chỉ của một con trỏ khác.
- dùng khi cần thay đổi nơi trỏ đến của con trỏ khi ở trong một hàm  
  Ex.
  ```c
  typedef struct linker_list_ {
    int val;
    struct linker_list_ *next;
  } linker_list;

  void init_list(linker_list** l) {
    *l = malloc(sizeof(linker_list*));
    (*l)->val = 0;
    (*l)->next = NULL;
  }

  int main() {
    linker_list *l = NULL;
    init_list(&l);
  }
  ```

## 3. So sánh dynamic memory allocation và static memory allocation, ý nghĩa của từng trường hợp
- static memory allocation
  - cấp phát bộ nhớ khi bắt đầu chương trình
  - thường được lưu trên Stack
  - tùy vào hệ điều hành nhưng dung lượng khá giới hạn khoảng 1-2MB
  - cấp phát đơn giản chỉ việc khai báo biến, chương trình khi chạy sẽ tự động cấp phát bộ nhớ cho những biến này
- dynamic memory allocation
  - cấp phát bộ nhớ ngay khi chương trình đang chạy
  - thường được lưu trên Heap
  - có thể cấp phát bộ nhớ theo yêu cầu của người dùng
  - có thể cấp phát được lượng lớn bộ nhớ
