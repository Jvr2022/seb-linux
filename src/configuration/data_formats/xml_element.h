#pragma once

#include <QString>
#include <QVariant>

namespace seb::configuration::data_formats {

struct XmlElement
{
    QString name;
    QVariant value;
};

}  // namespace seb::configuration::data_formats
