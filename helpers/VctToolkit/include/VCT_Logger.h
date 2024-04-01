// VCT_Logger.h

#pragma once

#include <ostream>
#include <ctime>
#include <sstream>
#include <string>

namespace vct
{

#pragma region Documentation
/// <summary>Message logging class for messages of various levels of severity, going to the prescribed destination. </summary>
/// <remarks>D. Higginbotham, 2017-02-24. </remarks>
#pragma endregion
class Logger
{
public:

    #pragma region Documentation
    /// <summary>Severity level testing rules (level >= stream or level matches stream). </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    #pragma endregion
    enum Rule
    {
        GREQUAL, /// Message types below log stream's level are ignored (default behavior).
        MATCHES  /// Message types which don't exactly match the log stream's level are ignored.
    };

    #pragma region Documentation
    /// <summary>Sets the message severity level rule. </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    /// <param name="test">Severity level rule enum (GREQUAL or MATCHES).</param>
    #pragma endregion
    void setSeverityLevelRule(Logger::Rule test)
    {
        rule = test;
    }

    #pragma region Documentation
    /// <summary>Sets the message severity level. </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    #pragma endregion
    enum Severity
    {
        LOG_ALL   = 0x00,   /// All messages will be output (regardless of current rule)
        LOG_INFO  = 0x01,  /// Informational messages only (no bearing on program execution)
        LOG_WARN  = 0x02,  /// Warning messages (eg: sub-optimal program execution)
        LOG_ERROR = 0x03,   /// Error messages (eg: program execution is affected negatively)
        LOG_FATAL = 0x04, /// Fatal Error messages (eg: program execution is to be terminated)
        LOG_NONE  = 0xff   /// Block all messages from being output (regardless of current rule)
    };

    #pragma region Documentation
    /// <summary>Sets the message severity level (message types below this level are ignored). </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    /// <param name="level">Message level (Severity) enum. </param>
    #pragma endregion
    void setSeverityLevel(Logger::Severity level)
    {
        if (level == LOG_ALL)         // All messages will be logged
        {
            info.activate(true);
            warn.activate(true);
            error.activate(true);
            fatal.activate(true);
        }
        else if (level == LOG_NONE)   // No messages will be logged
        {
            info.activate(false);
            warn.activate(false);
            error.activate(false);
            fatal.activate(false);
        }
        else if (rule == GREQUAL) // Message's level >= stream's level will be logged
        {
            info.activate(level <= LOG_INFO);
            warn.activate(level <= LOG_WARN);
            error.activate(level <= LOG_ERROR);
            fatal.activate(level <= LOG_FATAL);
        }
        else if (rule == MATCHES) // Message's level == stream's level will be logged
        {
            info.activate(level == LOG_INFO);
            warn.activate(level == LOG_WARN);
            error.activate(level == LOG_ERROR);
            fatal.activate(level == LOG_FATAL);
        }
    }

    #pragma region Documentation
    /// <summary>Clear the message severity level (all messages will be output). </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    /// <param name="level">Message level (Severity) enum. </param>
    #pragma endregion
    void clearSeverityLevel()
    {
        setSeverityLevel(LOG_INFO);
    }

    #pragma region Documentation
    /// <summary>Constructor (parametric). </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    /// <param name="out">An established output stream (may be any output stream type). </param>
    #pragma endregion
    Logger(std::ostream &out)
        : stream(out),
          info(out, "INFO"),
          warn(out, "WARN"),
          error(out, "ERROR"),
          fatal(out, "FATAL"),
          rule(GREQUAL)
    {
        setSeverityLevel(LOG_NONE);
    }

    #pragma region Documentation
    /// <summary>Constructor (parametric). </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    /// <param name="level">Message level (Severity) enum. </param>
    /// <param name="out">An established output stream (may be any output stream type). </param>
    #pragma endregion
    Logger(Severity level, std::ostream &out)
        : stream(out),
          info(out, "INFO"),
          warn(out, "WARN"),
          error(out, "ERROR"),
          fatal(out, "FATAL"),
          rule(GREQUAL)
    {
        setSeverityLevel(level);
    }    
    

    #pragma region Documentation
    /// <summary>Output the current date and time to the log stream. </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    #pragma endregion
    void timestamp()
    {
        std::time_t tt = std::time(NULL);
        std::string l_time(std::ctime(&tt));
        l_time.insert(l_time.size() - 1, "]:\n");
        stream << "\n[" << l_time;
    }

private:


    #pragma region Documentation
    /// <summary>(default)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>Constructor is private to prevent attempted instantiation without an output stream.</remarks>
    #pragma endregion
    Logger(); // effectively prevent default constructor


    #pragma region Documentation
    /// <summary>(copy)Constructor: initialize internal states. </summary>
    /// <parm name="rhs">A pre-existing instance of this class. </parm>
    /// <remarks>D. Higginbotham, 2017-04-21. </remarks>
    /// <remarks>Copy constructor is private to prevent attempted copying (due to non-copyable, underlying streams.</remarks>
    #pragma endregion
    Logger(const Logger &rhs); // effectively prevent copying

    

    #pragma region Documentation
    /// <summary>Internal Message logging class for messages going to the prescribed destination. </summary>
    /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
    #pragma endregion
    class Log
    {
    public:

        #pragma region Documentation
        /// <summary>Constructor (parametric). </summary>
        /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
        /// <param name="out">An established output stream (may be any output stream type). </param>
        /// <param name="cat">A string representing the type of log message. </param>
        #pragma endregion
        Log(std::ostream &out, std::string cat) : stream(out), category(cat), active(true)
        {}

        #pragma region Documentation
        /// <summary>Constructor (parametric). </summary>
        /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
        /// <param name="rhs">An existing instance of Log. </param>
        #pragma endregion
        Log(const Log &rhs) : stream(rhs.stream), category(rhs.category), active(rhs.active)
        {}

        #pragma region Documentation
        /// <summary>Overloaded output operator. </summary>
        /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
        /// <param name="item">A single stream object to be output (these can be combined as with any stream). </param>
        /// <return>The outputstream object.</return>
        #pragma endregion
        template <typename T>
        const Log &operator<<(const T &item) const
        {

            std::ostringstream oss;
            oss << item;
            std::string test(oss.str());
            stringout(test);
            return *this;
        }

        #pragma region Documentation
        /// <summary>Ouput a single encoded item string with message level appropriately. </summary>
        /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
        /// <param name="item">A single string object to be output. </param>
        #pragma endregion
        void stringout(const std::string &item) const
        {
            static bool label_it = true;
            if (active)
            {
                if (label_it)
                {
                    stream << category << " : " << item;
                    label_it = false;
                }
                else
                {
                    stream << item;
                }

                if (item.find("\n") != std::string::npos)
                {
                    label_it = true;
                }
            }
        }

        #pragma region Documentation
        /// <summary>Activate - or inactivate - this log stream. </summary>
        /// <remarks>D. Higginbotham, 2017-02-24. </remarks>
        /// <param name="state">When true, log stream becomes active, when false, inactive. </param>
        #pragma endregion
        void activate(bool state)
        {
            active = state;
        }

    private:

        std::ostream &stream; /// underlying stream (supplied by client using constructor)
        std::string category; /// string identifying type of log message
        bool active;          /// log stream state (active or inactive)
    };

    Rule rule; /// severity level matching rule (see Rule enum).
    std::ostream &stream;

public:

    Log info;  /// use this stream for logging Informative messages
    Log warn;  /// use this stream for logging Warning messages
    Log error; /// use this stream for logging Error messages
    Log fatal; /// use this stream for logging Fatal Error messages
};

} // namespace vct