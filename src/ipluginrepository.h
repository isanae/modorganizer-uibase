#ifndef MODORGANIZER_IPLUGINREPOSITORY_INCLUDED
#define MODORGANIZER_IPLUGINREPOSITORY_INCLUDED

#include <mutex>
#include <QList>
#include <QPair>
#include <filesystem>
#include "iplugin.h"

namespace MOBase
{

class IDownload
{
public:
  enum States
  {
    Stopped = 0,
    Running,
    Stopping,
    Finished
  };

  struct Stats
  {
    double bytesPerSecond;
    double progress;
  };

  struct Info
  {
    QString outputFile;
    QList<QPair<QString, QString>> headers;
    QString userAgent;
  };


  virtual ~IDownload() = default;

  virtual States state() const = 0;
  virtual Stats stats() const = 0;
  virtual QByteArray buffer() const = 0;
  virtual int httpCode() const = 0;
  virtual void stop() = 0;
};


class IDownloader
{
public:
  virtual ~IDownloader() = default;

  virtual std::shared_ptr<IDownload> add(
    const QUrl& url, const IDownload::Info& info={}) = 0;
};


class IRepositoryDownload
{
public:
  enum class States
  {
    None = 0,
    Downloading,
    Stopping,
    Finished,
    Errored
  };

  static constexpr int NoError = 0;

  IRepositoryDownload()
    : m_state(States::None), m_error(NoError)
  {
  }

  virtual ~IRepositoryDownload() = default;

  virtual void tick() = 0;
  virtual void stop() = 0;
  virtual double progress() const = 0;

  void setState(States s, int e=NoError)
  {
    std::scoped_lock lock(m_stateMutex);
    m_state = s;
    m_error = e;
  }

  States state() const
  {
    std::scoped_lock lock(m_stateMutex);
    return m_state;
  }

  int error() const
  {
    std::scoped_lock lock(m_stateMutex);
    return m_error;
  }

private:
  mutable std::mutex m_stateMutex;
  States m_state;
  int m_error;
};


class IPluginRepository : public QObject, public IPlugin
{
  Q_INTERFACES(IPlugin)

public:
  virtual bool canHandleDownload(const QString& what) const = 0;
  virtual QString downloadFilename(const QString& what) const = 0;
  virtual std::unique_ptr<IRepositoryDownload> download(
      const QString& what, IDownloader* d) = 0;
};

} // namespace MOBase

Q_DECLARE_INTERFACE(MOBase::IPluginRepository, "org.ModOrganizer.PluginRepository/1.0")

#endif // MODORGANIZER_IPLUGINREPOSITORY_INCLUDED
