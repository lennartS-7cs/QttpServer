#include "swagger.h"
#include "httpserver.h"

using namespace std;
using namespace qttp;

const char* Swagger::ACTION_NAME = "swagger";

Swagger::Swagger() :
  Action(),
  m_IsEnabled(true),
  m_Response()
{
  initialize();
}

Swagger::~Swagger()
{
}

const char* Swagger::getName() const
{
  return ACTION_NAME;
}

set<HttpPath> Swagger::getRoutes() const
{
  return {
           { HttpMethod::GET, "/swagger" }
  };
}

void Swagger::initialize()
{
  HttpServer* svr = HttpServer::getInstance();

  m_IsEnabled = svr->isSwaggerEnabled();
  if(!m_IsEnabled)
  {
    return;
  }

  const HttpServer::ServerInfo& info = svr->getServerInfo();
  const QString& version = info.version;

  m_Response = QJsonObject {
    { "swagger", "2.0" },
    { "info", QJsonObject {
        { "title", info.title },
        { "description", info.description },
        { "version", info.version },
        { "termsOfService", info.termsOfService },
        { "contact", QJsonObject {
            { "email", info.contactEmail }
          }},
        { "license", QJsonObject {
            { "name", info.licenseName },
            { "url", info.licenseUrl }
          }},
        { "company", QJsonObject {
            { "name", info.companyName },
            { "url", info.companyUrl }
          }},
      }},
    { "host", info.host },
    { "basePath", info.basePath },
    { "schemes", info.schemes },
    { "consumes", info.consumes },
    { "produces", info.produces }
  };

  QJsonObject definitions;
  QJsonObject paths;
  QJsonObject parameters;

  const QHash<QString, shared_ptr<const Action> >& actions = svr->getActions();

  // Holds the path and each input referenced string.
  std::map<HttpPath, QVector<QJsonObject> > inputLookup;

  for(const auto & action : actions)
  {
    QString actionName = action->getName();

    // First we get and correct, if needed,  the Swagger method
    // return values from the action
    auto prependSlashToMapKeys = [] (auto inputMap, auto &outputMap) {
      for(const auto input : inputMap)
      {
        auto key = input.first;
        if(!key.second.startsWith("/"))
        {
          key.second.prepend("/");
        }
        outputMap.insert({ key, input.second });
      }
    };
    std::map<qttp::HttpPath, const char*> actionSummaries;
    prependSlashToMapKeys(action->getSummaries(), actionSummaries);
    std::map<qttp::HttpPath, const char*> actionDescriptions;
    prependSlashToMapKeys(action->getDescriptions(), actionDescriptions);
    std::map<qttp::HttpPath, QStringList> actionTags;
    prependSlashToMapKeys(action->getTags(), actionTags);
    std::map<qttp::HttpPath, std::vector<qttp::Input>> actionInputs;
    prependSlashToMapKeys(action->getInputs(), actionInputs);
    std::map<qttp::HttpPath, std::map<qttp::HttpStatus, QString>> actionResponses;
    prependSlashToMapKeys(action->getResponses(), actionResponses);

    for(auto httpMethod : Global::HTTP_METHODS)
    {
      const QHash<QString, Route>& routes = svr->getRoutes(httpMethod);

      for(const auto & route : routes)
      {
        QJsonArray routeParameters;

        if(route.visibility == Visibility::Hide)
        {
          // If this is meant to be private then let's not list on SwaggerUI.
          continue;
        }

        if(route.action != actionName)
        {
          // A route with a blank action is usually the default base-route.
          continue;
        }

        HttpPath currentHttpPath = { route.method, route.path };
        QJsonObject properties;
        QJsonArray required;
        if(actionInputs.find(currentHttpPath) != actionInputs.end())
        {
          for(const Input &input : actionInputs[currentHttpPath])
          {
            if(input.visibility == Visibility::Hide)
            {
              // Obviously let's skip this if this is not meant to be publicly listed.
              continue;
            }

            QJsonArray enums;
            for(int i = 0; i < input.values.size(); ++i)
            {
              enums.append(input.values.at(i));
            }

            QString paramKey = "action_" + actionName + version + "_" + input.name;

            parameters[paramKey] = QJsonObject {
              { "name", input.name },
              { "in", input.paramType },
              { "type", input.dataType },
              { "description", input.description },
              { "required", input.isRequired },
              { "enum", enums.size() == 0 ? QJsonValue() : enums }
            };

            if(input.isRequired)
            {
              required.append(input.name);
            }

            auto iter = inputLookup.find(currentHttpPath);
            if(iter == inputLookup.end())
            {
              inputLookup.insert({currentHttpPath, QVector<QJsonObject>()});
            }
            inputLookup[currentHttpPath].append(QJsonObject {
              { "$ref", "#/parameters/" + paramKey }
            });

            properties[input.name] = QJsonObject { { "type", "string" } };
          }
        }

        definitions.insert(actionName + version, QJsonValue(QJsonObject {
          { "properties", properties }
        }));

        for(const QJsonObject & i : inputLookup[currentHttpPath])
        {
          routeParameters.append(i);
        }

        if(httpMethod == HttpMethod::POST ||
           httpMethod == HttpMethod::PUT ||
           httpMethod == HttpMethod::PATCH)
        {
          // TODO: Try and include the schema?

          routeParameters.append(QJsonObject {
            { "name", "body" },
            { "in", "body" },
            { "description", "Body of the action" },
            { "schema", QJsonObject { { "type", "object" } } }
          });
        }

        QString routePath;
        for(QString part : route.parts)
        {
          // If the parts of the route contains an input variable
          // (starts with ":"), then go ahead and replace to make it "{param}"
          // so that swagger can digest.

          if(part.startsWith(':'))
          {
            routePath.append("/{" + part.mid(1) + "}");
          }
          else if(!part.isEmpty())
          {
            routePath.append("/" + part);
          }
        }
        if(!paths.contains(routePath))
        {
          paths.insert(routePath, QJsonObject());
        }

        QJsonObject responses;
        if(actionResponses.find(currentHttpPath) != actionResponses.end())
        {
          for(auto response : actionResponses[currentHttpPath])
          {
            responses.insert(QString::number(static_cast<int>(response.first)), QJsonObject {
              { "description", response.second }
            });
          }
        }

        QJsonArray tags = QJsonArray::fromStringList(actionTags[currentHttpPath]);

        QJsonObject pathRoute = paths[routePath].toObject();
        pathRoute.insert(Utils::toStringLower(httpMethod), QJsonObject {
          { "summary", actionSummaries[currentHttpPath] },
          { "description", actionDescriptions[currentHttpPath] },
          { "operationId", routePath },
          { "parameters", routeParameters },
          { "tags", tags },
          { "responses", responses }
        });
        paths[routePath] = pathRoute;
      }
    }

  } // End for(const auto & action : action)

  //paths.insert(route.route, QJsonValue(path));

  m_Response["definitions"] = definitions;
  m_Response["paths"] = paths;
  m_Response["parameters"] = parameters;
}

void Swagger::onGet(HttpData& data)
{
  if(m_IsEnabled)
  {
    data.getResponse().setJson(m_Response);
  }
  else
  {
    // TODO: BUILD from some centralized class - Should have something more
    // uniform.
    data.setErrorResponse(QJsonObject {
      { "error", "Invalid request" }
    });
  }
}
