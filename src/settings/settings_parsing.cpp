#include "settings_parsing.h"

#include <QCryptographicHash>
#include <QDomDocument>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMetaType>
#include <QVariant>

#include <zlib.h>

namespace seb::settingsinternal {
namespace {

QJsonValue toJsonValue(const QVariant &value)
{
    if (!value.isValid()) return {};

    if (value.metaType().id() == QMetaType::QVariantMap) {
        QJsonObject object;
        const QVariantMap map = value.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            object.insert(it.key(), toJsonValue(it.value()));
        }
        return object;
    }

    if (value.metaType().id() == QMetaType::QVariantList) {
        QJsonArray array;
        const QVariantList list = value.toList();
        for (const QVariant &item : list) {
            array.push_back(toJsonValue(item));
        }
        return array;
    }

    if (value.metaType().id() == QMetaType::QByteArray) {
        return QString::fromLatin1(value.toByteArray().toBase64());
    }

    return QJsonValue::fromVariant(value);
}

QString sha256Hex(const QByteArray &data)
{
    return QString::fromLatin1(QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex());
}

QVariant parsePlistElement(const QDomElement &element)
{
    const QString tag = element.tagName();
    if (tag == "dict") {
        QVariantMap map;
        QDomElement child = element.firstChildElement();
        while (!child.isNull()) {
            if (child.tagName() != "key") {
                child = child.nextSiblingElement();
                continue;
            }

            const QString key = child.text();
            const QDomElement valueElement = child.nextSiblingElement();
            if (valueElement.isNull()) break;

            map.insert(key, parsePlistElement(valueElement));
            child = valueElement.nextSiblingElement();
        }
        return map;
    }

    if (tag == "array") {
        QVariantList values;
        for (QDomElement child = element.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
            values.push_back(parsePlistElement(child));
        }
        return values;
    }

    if (tag == "true") return true;
    if (tag == "false") return false;
    if (tag == "integer") {
        bool ok = false;
        const int value = element.text().trimmed().toInt(&ok);
        return ok ? QVariant(value) : QVariant(element.text().trimmed());
    }
    if (tag == "real") {
        bool ok = false;
        const double value = element.text().trimmed().toDouble(&ok);
        return ok ? QVariant(value) : QVariant(element.text().trimmed());
    }
    if (tag == "data") return QByteArray::fromBase64(element.text().trimmed().toUtf8());
    return element.text();
}

}  // namespace

QString calculateConfigurationKey(const QVariantMap &data)
{
    const QJsonObject object = toJsonValue(data).toObject();
    return sha256Hex(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

QByteArray inflateGzip(const QByteArray &data, QString *error)
{
    z_stream stream {};
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
    stream.avail_in = static_cast<uInt>(data.size());

    if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
        if (error) *error = QStringLiteral("Failed to initialize gzip decompression.");
        return {};
    }

    QByteArray output;
    char buffer[8192];
    int status = Z_OK;

    while (status == Z_OK) {
        stream.next_out = reinterpret_cast<Bytef *>(buffer);
        stream.avail_out = sizeof(buffer);
        status = inflate(&stream, Z_NO_FLUSH);

        if (status != Z_OK && status != Z_STREAM_END) {
            if (error) *error = QStringLiteral("Failed to decompress gzip-compressed SEB data.");
            inflateEnd(&stream);
            return {};
        }

        output.append(buffer, static_cast<int>(sizeof(buffer) - stream.avail_out));
    }

    inflateEnd(&stream);
    return output;
}

QVariantMap parsePlist(const QByteArray &raw, QString *error)
{
    QDomDocument document;
    QString parseError;
    int line = 0;
    int column = 0;
    if (!document.setContent(raw, &parseError, &line, &column)) {
        if (error) {
            *error = QStringLiteral("Failed to parse XML property list at line %1, column %2: %3")
                         .arg(line)
                         .arg(column)
                         .arg(parseError);
        }
        return {};
    }

    const QDomElement root = document.documentElement();
    if (root.tagName() != "plist") {
        if (error) *error = QStringLiteral("Unsupported XML format: expected a plist root element.");
        return {};
    }

    const QDomElement payload = root.firstChildElement();
    if (payload.isNull() || payload.tagName() != "dict") {
        if (error) *error = QStringLiteral("Unsupported plist payload: expected a top-level dict.");
        return {};
    }

    return parsePlistElement(payload).toMap();
}

QVariantMap parseJson(const QByteArray &raw, QString *error)
{
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(raw, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (error) *error = QStringLiteral("Failed to parse JSON: %1").arg(parseError.errorString());
        return {};
    }

    if (!document.isObject()) {
        if (error) *error = QStringLiteral("Expected a JSON object at the top level.");
        return {};
    }

    return document.object().toVariantMap();
}

}  // namespace seb::settingsinternal
