// src/utils/OsUtils.h
#pragma once
#include <QString>
#include <QDir>

namespace OsUtils {

inline QString executableName(const QString& name) {
#ifdef Q_OS_WIN
  return name + ".exe";
#else
  return name;
#endif
}

inline QString fileManagerCommand() {
#ifdef Q_OS_WIN
  return "explorer";
#elif defined(Q_OS_MAC)
  return "open";
#else
  return "xdg-open";
#endif
}

inline QString configDir() {
#ifdef Q_OS_WIN
  return qEnvironmentVariable("APPDATA");
#elif defined(Q_OS_MAC)
  return QDir::homePath() + "/Library/Application Support";
#else
  return qEnvironmentVariable("XDG_CONFIG_HOME", QDir::homePath() + "/.config");
#endif
}

} // namespace OsUtils
