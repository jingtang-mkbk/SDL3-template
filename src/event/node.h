#ifndef NODE_H
#define NODE_H

#include "base.h"

Node *Node_Create(void);
void Node_Default(Node *node, NodeType type);
void Node_AddChild(Node *parent, Node *child);

void Node_SetPosition(Node *node, float x, float y);
void Node_SetSize(Node *node, float w, float h);
void Node_SetAnchor(Node *node, float offsetX, float offsetY);
void Node_SetRotate(Node *node, double deg);
void Node_SetVisible(Node *node, bool visible);

/* 求 node 在以 root 为根的树中的世界坐标（沿祖先链累加 rect.x/y 偏移）；找不到返回 false */
bool Node_GetWorldRect(Node *root, Node *node, SDL_FRect *out);

#endif /* NODE_H */
