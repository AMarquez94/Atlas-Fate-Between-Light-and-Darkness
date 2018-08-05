#include "mcv_platform.h"
#include "btnode.h"
#include "comp_bt.h"

BTNode::BTNode(const std::string& btName)
{
    name = btName;
}

void BTNode::create(const std::string& btName) {
    name = btName;
}

bool BTNode::isRoot() {
    return parent == nullptr;
}

void BTNode::setParent(BTNode *newParent) {
    parent = newParent;
}

void BTNode::setRight(BTNode *newRight) {
    right = newRight;
}

void BTNode::addChild(BTNode *newChild) {
    if (!children.empty()) {

        /* If node already had children, last node connected to the new one */
        children.back()->setRight(newChild);
    }
    children.push_back(newChild);
    newChild->right = nullptr;
}

void BTNode::setType(EType type) {
    this->type = type;
}

void BTNode::update(float dt, TCompIAController *bt) {
    switch (type) {
    case EType::ACTION:
    {

        /* Run the controller of this node */

        /* Calculate what's the next node to use in the next frame */
        ERes res = bt->execAction(name, dt);
        bt->addActionToHistoric(name);  //TODO: delete - only for debug
        if (res == ERes::STAY) {
            if (bt->testAssert(name, dt)) {

                /* We test if the assert (if any) is true every step */
                bt->setCurrent(this);
            }
            else {

                /* Assert failed => reset */
                bt->setCurrent(nullptr);
            }
        }
        else if (res == ERes::LEAVE) {

            /* Climb tree iteratively and look for the next unfinished sequence to complete */
            BTNode *candidate = this;
            while (candidate->parent != nullptr) {
                BTNode *candParent = candidate->parent;
                if (candParent->type == EType::SEQUENCE) {
                    /* We were doing a sequence => finish it */

                    if (candidate->right != nullptr) {

                        /* Exec the right node to continue the sequence */
                        bt->setCurrent(candidate->right);
                        break;
                    }
                    else {
                        candidate = candParent;
                    }
                }
                else {

                    /* It is not a sequence => climb up to the root */
                    candidate = candParent;
                }
            }

            if (candidate->parent == nullptr) {

                /* If we reached the root, reset tree trace for the next frame*/
                bt->setCurrent(nullptr);
            }
        }
        else if (res == ERes::UNIMPLEMENTED) {
            fatal("ACTION UNIMPLEMENTED");
        }
        break;
    }
    case EType::RANDOM:
    {
        int r = rand() % children.size();
        children[r]->update(dt, bt);
        break;
    }
    case EType::PRIORITY:
    {
        for (int i = 0; i < children.size(); i++) {
            if (bt->testCondition(children[i]->getName(), dt)) {
                children[i]->update(dt, bt);
                break;
            }
        }
        break;
    }
    case EType::SEQUENCE:
    {
        /* Begin the sequence. First node will take care of the sequence */
        children[0]->update(dt, bt);
        break;
    }
    default:
    {
        fatal("Type not defined!");
        break;
    }
    }
}

const std::string BTNode::getName() {
    return this->name;
}

void BTNode::printNode(int level, TCompIAController *bt)
{
    std::string text = level == 0 ? "" : " ";
    for (int i = 0; i < level; i++) {
        text = text + "| ";
    }
    text = text + this->getName();
    switch (type) {
    case EType::ACTION:
        text = text + " (act.)";
        break;
    case EType::RANDOM:
        text = text + " (r)";
        break;
    case EType::PRIORITY:
        text = text + " (p)";
        break;
    case EType::SEQUENCE:
        text = text + " (s)";
        break;
    }
    if (bt->getCurrent() == this) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255.0f, 255.0f, 0.0f, 1.0f));
        ImGui::Text("%s", text.c_str());
        ImGui::PopStyleColor();
    }
    else {
        ImGui::Text("%s", text.c_str());
    }
    for (int i = 0; i < children.size(); i++) {
        children[i]->printNode(level + 1, bt);
    }
}