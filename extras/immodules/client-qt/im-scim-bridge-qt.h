#include <QtGlobal>
#if QT_VERSION >= 0x050000
#include <QObject>
#include <QtCore/QStringList>
#include <QtGui/qpa/qplatforminputcontextplugin_p.h>
#include <QtGui/qpa/qplatforminputcontext.h>
#elif QT_VERSION >= 0x040000
#include <Qt>
#include <QInputContextPlugin>
#else
#include <qinputcontextplugin.h>
#endif

#if QT_VERSION >= 0x050000
typedef QPlatformInputContext QInputContext;
typedef QPlatformInputContextPlugin QInputContextPlugin;
#elif QT_VERSION >= 0x040000
using namespace Qt;
#endif

/* The class Definition */
class ScimBridgeInputContextPlugin: public QInputContextPlugin
{
#if QT_VERSION >= 0x050000
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QPlatformInputContextFactoryInterface.5.1" FILE "scim.json")
#endif

    public:

        ScimBridgeInputContextPlugin ();

        ~ScimBridgeInputContextPlugin ();

#if QT_VERSION >= 0x050000
        QInputContext *create(const QString &key, const QStringList &paramList);
#else
    private:

        /**
         * The language list for SCIM.
         */
        static QStringList scim_languages;

    public:
        QStringList keys () const;

        QStringList languages (const QString &key);

        QString description (const QString &key);

        QString displayName (const QString &key);

        QInputContext *create (const QString &key);
#endif
};
