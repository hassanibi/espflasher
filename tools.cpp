#include "tools.h"

#include <QProcess>
#include <QFileInfo>
#include <QStringList>

namespace ESPFlasher {


Tools::Tools()
{

}

// Open Folder & select torrent's file or top folder
#undef HAVE_OPEN_SELECT
#if defined (Q_OS_WIN)
# define HAVE_OPEN_SELECT
static void Tools::openSelect (const QString& path)
{
  const QString explorer = QLatin1String ("explorer");
  QString param;
  if (!QFileInfo (path).isDir ())
    param = QLatin1String ("/select,");
  param += QDir::toNativeSeparators (path);
  QProcess::startDetached (explorer, QStringList (param));
}
#elif defined (Q_OS_MAC)
# define HAVE_OPEN_SELECT
static void Tools::openSelect (const QString& path)
{
  QStringList scriptArgs;
  scriptArgs << QLatin1String ("-e")
             << QString::fromLatin1 ("tell application \"Finder\" to reveal POSIX file \"%1\"").arg (path);
  QProcess::execute (QLatin1String ("/usr/bin/osascript"), scriptArgs);

  scriptArgs.clear ();
  scriptArgs << QLatin1String ("-e")
             << QLatin1String ("tell application \"Finder\" to activate");
  QProcess::execute (QLatin1String ("/usr/bin/osascript"), scriptArgs);
}
#endif

} //namespace ESPFlasher

