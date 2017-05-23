#ifndef MENUPROXYSTYLE_H
#define MENUPROXYSTYLE_H

#include <QtGui>
#include <QProxyStyle>

class MenuProxyStyle : public QProxyStyle
{
public:
    MenuProxyStyle(){}

    int styleHint(StyleHint hint, const QStyleOption *option = 0,
                                  const QWidget *widget = 0,
                                        QStyleHintReturn *returnData = 0) const
    {
        if (hint == QStyle::SH_DrawMenuBarSeparator)
            return QStyle::SH_DrawMenuBarSeparator;

        return QProxyStyle::styleHint(hint, option, widget, returnData);
     }
};


#endif // MENUPROXYSTYLE_H
