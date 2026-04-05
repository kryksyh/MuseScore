/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "accessibleapprootobject.h"

#include <QAccessible>
#include <QGuiApplication>

#include "accessibleapprootinterface.h"

#include "log.h"

using namespace muse::accessibility;

class AccessibilityActivationObserver : public QAccessible::ActivationObserver
{
public:
    AccessibilityActivationObserver()
        : m_active(QAccessible::isActive()) {}

    bool isActive() const { return m_active; }

    void accessibilityActiveChanged(bool active) override { m_active = active; }

private:
    bool m_active = false;
};

static AccessibilityActivationObserver* s_activationObserver = nullptr;

AccessibleAppRootObject::AccessibleAppRootObject()
    : QObject(nullptr)
{
    s_activationObserver = new AccessibilityActivationObserver();
    QAccessible::installActivationObserver(s_activationObserver);
}

void AccessibleAppRootObject::setup()
{
    if (m_setupDone) {
        return;
    }
    m_setupDone = true;

    QAccessible::installRootObjectHandler(nullptr);
    QAccessible::setRootObject(this);
}

AccessibleAppRootObject::~AccessibleAppRootObject()
{
    QAccessible::installActivationObserver(nullptr);
    delete s_activationObserver;
    s_activationObserver = nullptr;
}

QObject* AccessibleAppRootObject::asQObject()
{
    return this;
}

QAccessibleInterface* AccessibleAppRootObject::accessibleInterface(QObject* object)
{
    AccessibleAppRootObject* root = qobject_cast<AccessibleAppRootObject*>(object);
    if (!root) {
        return nullptr;
    }
    return new AccessibleAppRootInterface(root);
}

void AccessibleAppRootObject::setWindowRoot(AccessibleObject* root)
{
    m_windowRoot = root;
}

AccessibleObject* AccessibleAppRootObject::windowRoot() const
{
    return m_windowRoot;
}

void AccessibleAppRootObject::registerWindow(QWindow* window)
{
    if (!window) {
        return;
    }

    if (m_windows.contains(window)) {
        LOGW() << "Window already registered";
        return;
    }

    m_windows.append(window);
}

void AccessibleAppRootObject::unregisterWindow(QWindow* window)
{
    int i = m_windows.indexOf(window);
    if (i < 0) {
        return;
    }

    QAccessibleInterface* iface = QAccessible::queryAccessibleInterface(window);
    if (iface) {
        QAccessible::deleteAccessibleInterface(QAccessible::uniqueId(iface));
    }
    m_windows.removeAt(i);
}

int AccessibleAppRootObject::windowCount() const
{
    return m_windows.size();
}

QWindow* AccessibleAppRootObject::windowAt(int index) const
{
    if (index < 0 || index >= m_windows.size()) {
        return nullptr;
    }
    return m_windows[index];
}

QAccessibleInterface* AccessibleAppRootObject::windowIface(int index) const
{
    if (index < 0 || index >= m_windows.size()) {
        return nullptr;
    }
    return QAccessible::queryAccessibleInterface(m_windows[index]);
}

QAccessibleInterface* AccessibleAppRootObject::windowIface(QWindow* window) const
{
    if (!m_windows.contains(window)) {
        return nullptr;
    }
    return QAccessible::queryAccessibleInterface(window);
}

bool AccessibleAppRootObject::isAccessibilityActive() const
{
    return s_activationObserver && s_activationObserver->isActive();
}
