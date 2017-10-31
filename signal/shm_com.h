#define TEXT_SZ 2048
/*
当有数据写入这个结构中时，
我们用结构中的written_by_you标志来通知消费者。
需要传输的文本长度2K是随意定的
*/

struct shared_use_st {
  int written_by_you;
  char some_text[TEXT_SZ];
};
