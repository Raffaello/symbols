#pragma once

#include "Token.hpp"

#include <memory>
#include <list>

class AST
{
public:
    struct Node
    {
        Token token;

        std::unique_ptr<Node> l = nullptr;
        std::unique_ptr<Node> r = nullptr;

        Node() = default;
        Node(const Token& token);
    };

private:
    std::unique_ptr<Node> m_pRoot    = nullptr;
    Node*                 m_pCurrent = nullptr;

public:
    AST();
    ~AST();

    void add_left(const Token& token);
    void add_right(const Token& token);
    void move_left();
    void move_right();
};
