#pragma once

#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QKeyEvent>

// Property name an action sets to opt into "navigation" behaviour: when the
// action is triggered the owning StayOpenMenu rebuilds its own contents in
// place instead of closing like a normal menu.
inline constexpr char kStayOpenMenuNavProperty[] = "throne_stayOpenNav";

// A QMenu that does NOT close when a "navigation" action (one carrying the
// kStayOpenMenuNavProperty == true dynamic property) is clicked or activated by
// keyboard. Ordinary actions behave as usual and dismiss the menu.
//
// This lets a single menu act like a small paginated, navigable panel: the
// triggered slot mutates some state and rebuilds the menu (clear() + addAction)
// while it stays open. Rebuilds MUST be deferred (e.g. QTimer::singleShot(0, ...))
// so the just-triggered action is not deleted from inside its own trigger().
//
// Caveat: this relies on Qt painting the popup itself (Windows / Linux). On
// macOS a tray menu is a native NSMenu and these handlers are never invoked, so
// callers should keep a plain nested-submenu fallback there.
class StayOpenMenu : public QMenu {
public:
    using QMenu::QMenu;
    bool m_sticky = false;

    // When sticky, the menu does not collapse just because the cursor drifts off
    // it. As an open submenu it owns the mouse grab and receives every move, so
    // any move that falls outside its own rect is swallowed: this stops the
    // parent menu from deselecting and hiding it the instant the cursor leaves —
    // which is especially easy to trigger when the menu is short and most of the
    // surrounding area belongs to the parent. It still dismisses normally on a
    // click outside (press events are never swallowed), on Esc, or when a regular
    // (non-navigation) item is chosen.
    void setSticky(bool sticky) { m_sticky = sticky; }

    bool isSticky() const { return m_sticky; }

protected:
    void mouseMoveEvent(QMouseEvent *e) override {
        if (m_sticky && !rect().contains(e->pos())) {
            e->accept(); // cursor left the menu — keep it open, swallow the move
            return;
        }
        QMenu::mouseMoveEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent *e) override {
        if (QAction *act = actionAt(e->pos());
            act && act->isEnabled() && act->property(kStayOpenMenuNavProperty).toBool()) {
            act->trigger();  // the slot only flips state + schedules a deferred rebuild
            e->accept();     // swallow the release so the menu stays open
            return;
        }
        QMenu::mouseReleaseEvent(e);
    }

    void keyPressEvent(QKeyEvent *e) override {
        switch (e->key()) {
            case Qt::Key_Return:
            case Qt::Key_Enter:
            case Qt::Key_Space:
                if (QAction *act = activeAction();
                    act && act->isEnabled() && act->property(kStayOpenMenuNavProperty).toBool()) {
                    act->trigger();
                    e->accept();
                    return;
                }
                break;
            default:
                break;
        }
        QMenu::keyPressEvent(e);
    }
};
