#ifndef UI_REGISTRY_H
#define UI_REGISTRY_H

#include "event/mouse_event.h" /* Node 类型 */

/* 控件创建成功后自动登记到 ui 全局注册表（create 内部调用，勿手动重复登记） */
void ui_register(Node *node);   /* 按 node->type 分发到 img/spr/txt 注册表 */
void ui_unregister(Node *node); /* 从注册表移除（配合各控件 deinit 提前释放） */

#endif /* UI_REGISTRY_H */
