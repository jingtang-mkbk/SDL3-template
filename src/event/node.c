#include "node.h"

Node *Node_Create(void)
{
    return (Node *)SDL_calloc(1, sizeof(Node));
}

void Node_Default(Node *node, NodeType type)
{
    node->type = type;
    node->alpha = 1.0f;
    node->anchor = (SDL_FPoint){ 0.0f, 0.0f };
    node->angle = 0.0f;
    node->children = NULL;
    node->next = NULL;
    node->visible = true;
    node->z_index = 0;
}

/* 把 child 追加到 parent 的孩子链表尾部（保持插入顺序） */
void Node_AddChild(Node *parent, Node *child)
{
    if (!parent || !child)
        return;

    if (!parent->children) {
        parent->children = child;
        return;
    }
    Node *tail = parent->children;
    while (tail->next)
        tail = tail->next;
    tail->next = child;
}

void Node_SetPosition(Node *node, float x, float y)
{
    if (!node)
        return;

    node->rect.x = x;
    node->rect.y = y;
}

void Node_SetSize(Node *node, float w, float h)
{
    if (!node)
        return;

    node->rect.w = w;
    node->rect.h = h;
}

void Node_SetAnchor(Node *node, float offsetX, float offsetY)
{
    if (!node)
        return;

    node->anchor.x = node->rect.x + offsetX;
    node->anchor.y = node->rect.y + offsetY;
}

void Node_SetRotate(Node *node, double deg)
{
    if (!node)
        return;

    node->angle = deg;
}

void Node_SetVisible(Node *node, bool visible)
{
    if (!node)
        return;

    if (node->visible != visible)
        node->visible = visible;
}

static bool find_world_rect(Node *n, Node *target, float ox, float oy, SDL_FRect *out)
{
    if (!n)
        return false;

    float x = n->rect.x + ox;
    float y = n->rect.y + oy;
    if (n == target) {
        out->x = x;
        out->y = y;
        out->w = n->rect.w;
        out->h = n->rect.h;
        return true;
    }
    for (Node *c = n->children; c; c = c->next)
        if (find_world_rect(c, target, x, y, out))
            return true;
    return false;
}

/* 求 node 在以 root 为根的树中的世界坐标（沿祖先链累加 rect.x/y 偏移） */
bool Node_GetWorldRect(Node *root, Node *node, SDL_FRect *out)
{
    if (!root || !node || !out)
        return false;

    return find_world_rect(root, node, 0.0f, 0.0f, out);
}
