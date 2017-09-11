#ifndef QTTPACTION_H
#define QTTPACTION_H

#include <map>

#include "qttp_global.h"
#include "httproute.h"
#include "httpdata.h"

namespace qttp
{

/**
 * @brief Borrowing ideas from Node.js frameworks like Actionhero.js, an action
 * is ultimately the endpoint of the http request.
 */
class QTTPSHARED_EXPORT Action
{
  friend class HttpServer;

  public:

    Action();
    virtual ~Action();

    /**
     * @brief Primary entry point for processing actions. Default implementation
     * will invoke corresponding members against the HTTP methods: onGet(),
     * onPost(), onPut(), and onDelete()
     * @param data
     */
    virtual void onAction(HttpData& data);
    virtual void onGet(HttpData& data);
    virtual void onPost(HttpData& data);
    virtual void onPut(HttpData& data);
    virtual void onPatch(HttpData& data);
    virtual void onHead(HttpData& data);
    virtual void onDelete(HttpData& data);
    virtual void onOptions(HttpData& data);
    virtual void onTrace(HttpData& data);
    virtual void onConnect(HttpData& data);
    virtual void onUnknown(HttpData& data);

    /**
     * @brief Override  in order to associate this action to a specific
     * HttpMethod and path (e.g. "/myroute/").
     */
    virtual std::set<qttp::HttpPath> getRoutes() const;

    //! Returns the unique name for this action.
    virtual const char* getName() const = 0;

    //! The summary about this action - used for SwaggerUI.
    virtual std::map<qttp::HttpPath, const char*> getSummaries() const;

    //! The description about this action - used for SwaggerUI.
    virtual std::map<qttp::HttpPath, const char*> getDescriptions() const;

    //! The tags help SwaggerUI organize and group actions, like hashtags/labels.
    virtual std::map<qttp::HttpPath, QStringList> getTags() const;

    //! The inputs help SwaggerUI include parameters.
    virtual std::map<qttp::HttpPath, std::vector<Input>> getInputs() const;

    //! The possible responses of this action - used for SwaggerUI.
    virtual std::map<qttp::HttpPath, std::map<qttp::HttpStatus, QString>> getResponses() const;

    bool registerRoute(HttpMethod method, const QString& path, Visibility visibility = Visibility::Show);
    bool registerRoute(const qttp::HttpPath& path, Visibility visibility = Visibility::Show);
    void registerRoute(const std::vector<qttp::HttpPath>& routes, Visibility visibility = Visibility::Show);

QTTP_PROTECTED:

    /**
     * @brief Override in order to return a list of headers to automatically
     * append to each response.  This returns a reference to a vector because
     * this can be called frequently when assembling the outbound response.
     */
    virtual std::vector<QStringPair> getHeaders() const;

    /**
     * @brief Helps apply, modify, or prune headers in each response.
     */
    virtual void applyHeaders(HttpData& data) const;

QTTP_PRIVATE:

    static const std::set<qttp::HttpPath> EMPTY_ROUTES;
    static const std::map<qttp::HttpPath, std::vector<Input>> EMPTY_INPUTS;
    static const std::map<qttp::HttpPath, QStringList> EMPTY_TAG_LIST;
    static const std::vector<QStringPair> EMPTY_STRINGPAIR_LIST;
};

/**
 * @brief A Simple action that stores a basic call-back.  This will allow
 * us to supplement with additional information.
 */
class QTTPSHARED_EXPORT SimpleAction : public Action
{
  friend class HttpServer;

  public:

    SimpleAction(std::function<void(qttp::HttpData&)> callback);
    virtual ~SimpleAction();

    void onAction(HttpData& data);

    const char* getName() const;

    void setRoutes(const std::set<qttp::HttpPath>& routes);
    std::set<qttp::HttpPath> getRoutes() const;

    void setSummaries(std::map<qttp::HttpPath, const char*> summaries);
    std::map<qttp::HttpPath, const char*> getSummaries() const;

    void setDescriptions(std::map<qttp::HttpPath, const char*> descriptions);
    std::map<qttp::HttpPath, const char*> getDescriptions() const;

    void setTags(const std::map<qttp::HttpPath, QStringList>& tags);
    std::map<qttp::HttpPath, QStringList> getTags() const;
    void setInputs(const std::map<qttp::HttpPath, std::vector<Input>>& inputs);
    std::map<qttp::HttpPath, std::vector<Input>> getInputs() const;

    void setResponses(const std::map<qttp::HttpPath, std::map<qttp::HttpStatus, QString>> responses);
    std::map<qttp::HttpPath, std::map<qttp::HttpStatus, QString>> getResponses() const;

QTTP_PROTECTED:

    std::vector<QStringPair> getHeaders() const;

QTTP_PRIVATE:

    std::function<void(qttp::HttpData&)> m_Callback;
    QByteArray m_Name;
    std::set<qttp::HttpPath> m_Routes;
    std::map<qttp::HttpPath, QByteArray> m_Summaries;
    std::map<qttp::HttpPath, QByteArray> m_Descriptions;
    std::map<qttp::HttpPath, QStringList> m_Tags;
    std::map<qttp::HttpPath, std::vector<Input>> m_Inputs;
    std::map<qttp::HttpPath, std::map<qttp::HttpStatus, QString>> m_Responses;
    std::vector<QStringPair> m_Headers;
};

/**
 * @brief Like an interceptor, filter or a pipeline design pattern, this
 * processor is available for frequent and commonly performed tasks.  Again,
 * borrows from the node.js misnomer - known as a "middleware".
 */
class QTTPSHARED_EXPORT Processor
{
  public:

    Processor();
    virtual ~Processor();

    virtual const char* getName() const = 0;

    /// @brief Invoked before qttp::Action::onAction().
    virtual void preprocess(HttpData& data);

    /// @brief Invoked after qttp::Action::onAction().
    virtual void postprocess(HttpData& data);
};

} // End namespace qttp

#endif // QTTPACTION_H
