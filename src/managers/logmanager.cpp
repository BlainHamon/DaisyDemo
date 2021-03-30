#include "common.hpp"
#include "manager.hpp"
#include "logger.hpp"
#include "typedqueue.hpp"

enum LogProperties
{
  LogNoProperties = 0x00,
  LogShouldFree = 0x01,
  LogIncludeNewLine = 0x02,
};

struct LogEntry {
public:
  LogProperties props;
  char *text;
};

TypedQueue<LogEntry> logQueue(10);

void writeLog(char * text) {
  if (text == NULL) {
    return;
  }
  LogEntry entry;
  entry.props = LogNoProperties;
  entry.text = text;
  logQueue.send(entry);
}

void writeLogln(char * text){
  LogEntry entry;
  entry.props = LogIncludeNewLine;
  entry.text = text;
  logQueue.send(entry);
}

void writeLog(String *text) {
  writeLog(text->c_str());
}

void writeLogln(String *text) {
  writeLogln(text->c_str());
}

void writeLogf(const char *format, ...)
{
  LogEntry entry;
  char *message = (char*)malloc(80 * sizeof(char));
  va_list ap;
  va_start(ap, format);
  int result = vsprintf(message, format, ap);
  va_end(ap);
  if (result > 0) {
    entry.props = LogShouldFree;
    entry.text = message;
    logQueue.send(entry);
  }
}

class LogManager: public Manager {
public:
  LogManager();
  void setup();
  void loop();
  void handleEntry(LogEntry entry);
};

LogManager::LogManager()
{

}

void LogManager::handleEntry(LogEntry entry) {
  if (entry.text != NULL) {
    Serial.print(entry.text);
  }
  if (entry.props & LogIncludeNewLine) {
    Serial.println();
  }
  if (entry.props & LogShouldFree) {
    free(entry.text);
  }
}

void LogManager::setup()
{
  Serial.println("Starting up log");
}

void LogManager::loop()
{
  LogEntry entry = logQueue.receive();
  handleEntry(entry);
}

LogManager logmanager;
