#include "include/configs/outbounds/mieru.h"

#include <QJsonArray>
#include <QUrlQuery>
#include <include/global/Utils.hpp>

#include "include/configs/common/utils.h"

namespace Configs {
    namespace {
        // Split the user-entered comma-separated port ranges into a JSON string array.
        QJsonArray splitServerPorts(const QString& raw) {
            QJsonArray arr;
            for (auto part : raw.split(',', Qt::SkipEmptyParts)) {
                part = part.trimmed();
                if (!part.isEmpty()) arr.append(part);
            }
            return arr;
        }
    }

    bool mieru::ParseFromLink(const QString& link)
    {
        auto url = QUrl(link);
        if (!url.isValid()) return false;
        auto query = QUrlQuery(url.query());

        outbound::ParseFromLink(link);
        username = url.userName();
        password = url.password();

        if (query.hasQueryItem("transport")) transport = query.queryItemValue("transport").toUpper();
        if (query.hasQueryItem("multiplexing")) multiplexing = query.queryItemValue("multiplexing");
        if (query.hasQueryItem("traffic_pattern")) traffic_pattern = query.queryItemValue("traffic_pattern");
        if (query.hasQueryItem("server_ports")) server_ports = query.queryItemValue("server_ports");

        return true;
    }

    bool mieru::ParseFromJson(const QJsonObject& object)
    {
        if (object.isEmpty() || object["type"].toString() != "mieru") return false;
        outbound::ParseFromJson(object);
        if (object.contains("transport")) transport = object["transport"].toString();
        if (object.contains("username")) username = object["username"].toString();
        if (object.contains("password")) password = object["password"].toString();
        if (object.contains("multiplexing")) multiplexing = object["multiplexing"].toString();
        if (object.contains("traffic_pattern")) traffic_pattern = object["traffic_pattern"].toString();
        if (object.contains("server_ports")) {
            QStringList ports;
            for (const auto& v : object["server_ports"].toArray()) ports << v.toString();
            server_ports = ports.join(",");
        }
        return true;
    }

    bool mieru::ParseFromClash(const clash::Proxies& object)
    {
        // Clash does not support the mieru protocol.
        return false;
    }

    QString mieru::ExportToLink()
    {
        QUrl url;
        QUrlQuery query;
        url.setScheme("mieru");
        url.setUserName(username);
        url.setPassword(password);
        url.setHost(server);
        url.setPort(server_port);
        if (!name.isEmpty()) url.setFragment(name);

        if (!transport.isEmpty()) query.addQueryItem("transport", transport);
        if (!multiplexing.isEmpty()) query.addQueryItem("multiplexing", multiplexing);
        if (!traffic_pattern.isEmpty()) query.addQueryItem("traffic_pattern", traffic_pattern);
        if (!server_ports.isEmpty()) query.addQueryItem("server_ports", server_ports);

        mergeUrlQuery(query, outbound::ExportToLink());

        if (!query.isEmpty()) url.setQuery(query);
        return url.toString(QUrl::FullyEncoded);
    }

    QJsonObject mieru::ExportToJson()
    {
        QJsonObject object;
        object["type"] = "mieru";
        mergeJsonObjects(object, outbound::ExportToJson());
        object["transport"] = transport.isEmpty() ? QString("TCP") : transport;
        if (!username.isEmpty()) object["username"] = username;
        if (!password.isEmpty()) object["password"] = password;
        if (!multiplexing.isEmpty()) object["multiplexing"] = multiplexing;
        if (!traffic_pattern.isEmpty()) object["traffic_pattern"] = traffic_pattern;
        auto ports = splitServerPorts(server_ports);
        if (!ports.isEmpty()) object["server_ports"] = ports;
        return object;
    }

    BuildResult mieru::Build()
    {
        QJsonObject object;
        object["type"] = "mieru";
        mergeJsonObjects(object, outbound::Build().object);
        object["transport"] = transport.isEmpty() ? QString("TCP") : transport;
        if (!username.isEmpty()) object["username"] = username;
        if (!password.isEmpty()) object["password"] = password;
        if (!multiplexing.isEmpty()) object["multiplexing"] = multiplexing;
        if (!traffic_pattern.isEmpty()) object["traffic_pattern"] = traffic_pattern;
        auto ports = splitServerPorts(server_ports);
        if (!ports.isEmpty()) object["server_ports"] = ports;
        return {object, ""};
    }

    QString mieru::DisplayType()
    {
        return "Mieru";
    }
}
