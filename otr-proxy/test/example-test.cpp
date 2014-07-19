#include <QtCore>
#include <QtTest>

class ExampleTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testDummy()
    {
        QVERIFY(true);
    }
};

QTEST_MAIN(ExampleTest)
#include "example-test.moc"
