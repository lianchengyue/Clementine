#include "mock_networkaccessmanager.h"

#include <QtDebug>

#include <algorithm>
using std::min;

using ::testing::MakeMatcher;
using ::testing::Matcher;
using ::testing::MatcherInterface;
using ::testing::Return;

class RequestForUrlMatcher : public MatcherInterface<const QNetworkRequest&> {
 public:
  RequestForUrlMatcher(const QString& contains,
                       const QMap<QString, QString>& expected_params)
      : contains_(contains),
        expected_params_(expected_params) {
  }
  virtual ~RequestForUrlMatcher() {}

  virtual bool Matches(const QNetworkRequest& req) const {
    const QUrl& url = req.url();

    if (!url.toString().contains(contains_)) {
      return false;
    }

    for (QMap<QString, QString>::const_iterator it = expected_params_.constBegin();
         it != expected_params_.constEnd(); ++it) {
      if (!url.hasQueryItem(it.key()) ||
          url.queryItemValue(it.key()) != it.value()) {
        return false;
      }
    }
    return true;
  }

  virtual void DescribeTo(::std::ostream* os) const {
    *os << "matches url";
  }

 private:
  QString contains_;
  QMap<QString, QString> expected_params_;
};

inline Matcher<const QNetworkRequest&> RequestForUrl(
    const QString& contains,
    const QMap<QString, QString>& params) {
  return MakeMatcher(new RequestForUrlMatcher(contains, params));
}

MockNetworkReply* MockNetworkAccessManager::ExpectGet(
    const QString& contains,
    const QMap<QString, QString>& expected_params,
    int status,
    const QByteArray& data) {
  MockNetworkReply* reply = new MockNetworkReply(data);
  reply->setAttribute(QNetworkRequest::HttpStatusCodeAttribute, status);

  EXPECT_CALL(*this, createRequest(
      GetOperation, RequestForUrl(contains, expected_params), NULL)).
          WillOnce(Return(reply));

  return reply;
}

MockNetworkReply::MockNetworkReply()
    : data_(NULL) {
}

MockNetworkReply::MockNetworkReply(const QByteArray& data)
    : data_(data),
      pos_(0) {
}

void MockNetworkReply::SetData(const QByteArray& data) {
  data_ = data;
  pos_ = 0;
}

qint64 MockNetworkReply::readData(char* data, qint64 size) {
  if (data_.size() == pos_) {
    return -1;
  }
  qint64 bytes_to_read = min(data_.size() - pos_, size);
  memcpy(data, data_.constData() + pos_, bytes_to_read);
  pos_ += bytes_to_read;
  return bytes_to_read;
}

qint64 MockNetworkReply::writeData(const char* data, qint64) {
  ADD_FAILURE() << "Something tried to write to a QNetworkReply";
}

void MockNetworkReply::Done() {
  setOpenMode(QIODevice::ReadOnly);
  emit finished();
}

void MockNetworkReply::setAttribute(QNetworkRequest::Attribute code, const QVariant& value) {
  QNetworkReply::setAttribute(code, value);
}
