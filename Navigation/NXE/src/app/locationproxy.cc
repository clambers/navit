#include "locationproxy.h"

LocationProxy::LocationProxy(QString itemText, bool fav, QString desc, bool bolded, QObject *parent)
    : QObject(parent)
    , _itemText(itemText)
    , _favorite(fav)
    , _description(desc)
    , _bolded(bolded)
{
}

void LocationProxy::setFavorite(bool bFav)
{

}

void LocationProxy::setDescription(const QString &desc)
{
}

void LocationProxy::setBolded(bool b)
{
}

