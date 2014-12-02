/*
 * Copyright (C) 2013 Dan Vrátil <dvratil@redhat.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "contact-info-dialog.h"
#include "contact.h"

#include <QGridLayout>
#include <QPicture>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>

#include <TelepathyQt/Contact>
#include <TelepathyQt/PendingContactInfo>
#include <TelepathyQt/AvatarData>
#include <TelepathyQt/Presence>
#include <TelepathyQt/SharedPtr>
#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Connection>
#include <TelepathyQt/Account>
#include <TelepathyQt/PendingContacts>
#include <TelepathyQt/PendingReady>

#include <QDebug>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QMimeType>
#include <QMimeDatabase>
#include <QDialogButtonBox>

#include <KMessageBox>
#include <KTitleWidget>
#include <KLocalizedString>
#include <KDateComboBox>
#include <KImageFilePreview>
#include <KIconLoader>

namespace KTp {

enum InfoRowIndex {
    FullName = 0,
    Nickname,
    Email,
    Phone,
    Homepage,
    Birthday,
    Organization,
    _InfoRowCount
};

static struct InfoRow {
    const InfoRowIndex index;
    const QString fieldName;
    const char* title;
} InfoRows[] = {                                        // Don't use i18n in global static vars
    { FullName,         QLatin1String("fn"),            I18N_NOOP("Full name:") },
    { Nickname,         QLatin1String("nickname"),      I18N_NOOP("Nickname:") },
    { Email,            QLatin1String("email"),         I18N_NOOP("Email:") },
    { Phone,            QLatin1String("tel"),           I18N_NOOP("Phone:") },
    { Homepage,         QLatin1String("url"),           I18N_NOOP("Homepage:") },
    { Birthday,         QLatin1String("bday"),          I18N_NOOP("Birthday:") },
    { Organization,     QLatin1String("org"),           I18N_NOOP("Organization:") }
};

class ContactInfoDialog::Private
{
  public:
    Private(ContactInfoDialog *parent):
        editable(false),
        infoDataChanged(false),
        avatarChanged(false),
        columnsLayout(0),
        infoLayout(0),
        stateLayout(0),
        changeAvatarButton(0),
        clearAvatarButton(0),
        avatarLabel(0),
        q(parent)
    {}

    void onContactUpgraded(Tp::PendingOperation *op);
    void onContactInfoReceived(Tp::PendingOperation *op);
    void onChangeAvatarButtonClicked();
    void onClearAvatarButtonClicked();
    void onInfoDataChanged();
    void onFeatureRosterReady(Tp::PendingOperation *op);

    void addInfoRow(InfoRowIndex index, const QString &value);
    void addStateRow(const QString &description, Tp::Contact::PresenceState state);
    void loadStateRows();

    Tp::AccountPtr account;
    KTp::ContactPtr contact;
    bool editable;

    bool infoDataChanged;
    bool avatarChanged;
    QString newAvatarFile;

    QMap<InfoRowIndex,QWidget*> infoValueWidgets;

    QHBoxLayout *columnsLayout;
    QFormLayout *infoLayout;
    QFormLayout *stateLayout;
    QPushButton *changeAvatarButton;
    QPushButton *clearAvatarButton;
    QLabel *avatarLabel;
    QDialogButtonBox *buttonBox;

  private:
    ContactInfoDialog *q;
};

void ContactInfoDialog::Private::onContactUpgraded(Tp::PendingOperation* op)
{
    Tp::PendingContacts *contacts = qobject_cast<Tp::PendingContacts*>(op);
    if (op->isError()) {
        return;
    }

    Q_ASSERT(contacts->contacts().count() == 1);

    contact = KTp::ContactPtr::qObjectCast(contacts->contacts().first());

    /* Show avatar immediatelly */
    if (contacts->features().contains(Tp::Contact::FeatureAvatarData)) {
        QVBoxLayout *avatarLayout = new QVBoxLayout();
        avatarLayout->setSpacing(5);
        avatarLayout->setAlignment(Qt::AlignHCenter);
        columnsLayout->addLayout(avatarLayout);

        avatarLabel = new QLabel(q);
        avatarLabel->setMaximumSize(150, 150);
        avatarLayout->addWidget(avatarLabel, 0, Qt::AlignTop);

        if (editable) {
            changeAvatarButton = new QPushButton(i18n("Change Avatar"), q);
            connect(changeAvatarButton, SIGNAL(clicked(bool)),
                    q, SLOT(onChangeAvatarButtonClicked()));
            avatarLayout->addWidget(changeAvatarButton);

            clearAvatarButton = new QPushButton(i18n("Clear Avatar"), q);
            connect(clearAvatarButton, SIGNAL(clicked(bool)),
                    q, SLOT(onClearAvatarButtonClicked()));
            avatarLayout->addWidget(clearAvatarButton);

            avatarLayout->addStretch(1);
        }

        QPixmap avatar(contact->avatarPixmap());
        avatarLabel->setPixmap(avatar.scaled(avatarLabel->maximumSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    /* Request detailed contact info */
    if (contacts->features().contains(Tp::Contact::FeatureInfo)) {
        infoLayout = new QFormLayout();
        infoLayout->setSpacing(10);
        columnsLayout->addLayout(infoLayout);

        Tp::PendingContactInfo *op = contact->requestInfo();
        connect(op, SIGNAL(finished(Tp::PendingOperation*)),
                q, SLOT(onContactInfoReceived(Tp::PendingOperation*)));
    }
}

void ContactInfoDialog::Private::onFeatureRosterReady(Tp::PendingOperation *op)
{
    loadStateRows();
}

void ContactInfoDialog::Private::onContactInfoReceived(Tp::PendingOperation* op)
{
    Tp::PendingContactInfo *ci = qobject_cast<Tp::PendingContactInfo*>(op);
    const Tp::ContactInfoFieldList fieldList = ci->infoFields().allFields();

    for (InfoRowIndex index = (InfoRowIndex) 0; index < _InfoRowCount; index = (InfoRowIndex)(index + 1)) {
        QString value;

        Q_FOREACH(const Tp::ContactInfoField &field, fieldList) {
            if (field.fieldValue.count() == 0) {
                continue;
            }

            if (field.fieldName == InfoRows[index].fieldName) {
                value = field.fieldValue.first();
                break;
            }
        }

        /* Show edits for all values when in editable mode */
        if (!editable && value.isEmpty()) {
            continue;
        }

        addInfoRow(index, value);
    }
}

void ContactInfoDialog::Private::onChangeAvatarButtonClicked()
{
    QPointer<QFileDialog> fileDialog = new QFileDialog(q);
//     fileDialog->setPreviewWidget(new KImageFilePreview(fileDialog)); //TODO KF5 - is there a replacement?
    fileDialog->setMimeTypeFilters(QStringList() << QStringLiteral("image/*"));
    fileDialog->setFileMode(QFileDialog::ExistingFile);

    int c = fileDialog->exec();
    if (fileDialog && c && !fileDialog->selectedFiles().isEmpty()) {
        newAvatarFile = fileDialog->selectedFiles().first();

        QPixmap avatar(newAvatarFile);
        if (avatar.isNull()) {
            KMessageBox::error(q, i18n("Failed to load the new avatar image"));
            newAvatarFile.clear();
            delete fileDialog;
            return;
        }
        avatarLabel->setPixmap(avatar.scaled(avatarLabel->maximumSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        avatarChanged = true;
        clearAvatarButton->setEnabled(true);
    }

    delete fileDialog;
}

void ContactInfoDialog::Private::onClearAvatarButtonClicked()
{
    QPixmap avatar;
    avatar = KIconLoader::global()->loadIcon(QLatin1String("im-user"), KIconLoader::Desktop, 128);

    newAvatarFile.clear();
    avatarChanged = true;
}

void ContactInfoDialog::Private::onInfoDataChanged()
{
    infoDataChanged = true;
}

void ContactInfoDialog::Private::addInfoRow(InfoRowIndex index, const QString &value)
{
    InfoRow *row = &InfoRows[index];

    // I18N_NOOP only marks the string for translation, the actual lookup of
    // translated row->title happens here
    QLabel *descriptionLabel = new QLabel(i18n(row->title), q);
    QFont font = descriptionLabel->font();
    font.setBold(true);
    descriptionLabel->setFont(font);

    if (editable) {
        if (index == Birthday) {
            KDateComboBox *combo = new KDateComboBox(q);
            combo->setOptions(KDateComboBox::EditDate | KDateComboBox::SelectDate | KDateComboBox::DatePicker);
            combo->setMinimumWidth(200);
            combo->setDate(QDate::fromString(value));
            connect(combo, SIGNAL(dateChanged(QDate)), q, SLOT(onInfoDataChanged()));

            infoValueWidgets.insert(index, combo);
        } else {
            QLineEdit *edit = new QLineEdit(q);
            edit->setMinimumWidth(200);
            edit->setText(value);
            connect(edit, SIGNAL(textChanged(QString)), q, SLOT(onInfoDataChanged()));

            infoValueWidgets.insert(index, edit);
        }
    } else {
        QLabel *label = new QLabel(q);
        label->setOpenExternalLinks(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        if (index == Email) {
            label->setText(QString::fromLatin1("<a href=\"mailto:%1\">%1</a>").arg(value));
        } else if (index == Homepage) {
            QString format;
            if (!value.startsWith(QLatin1String("http"), Qt::CaseInsensitive)) {
                format = QLatin1String("<a href=\"http://%1\">%1</a>");
            } else {
                format = QLatin1String("<a href=\"%1\">%1</a>");
            }
            label->setText(format.arg(value));
        } else {
            label->setText(value);
        }

        infoValueWidgets.insert(index, label);
    }

    infoLayout->addRow(descriptionLabel, infoValueWidgets.value(index));
}

void ContactInfoDialog::Private::addStateRow(const QString& description, Tp::Contact::PresenceState state)
{
    QLabel *descriptionLabel = new QLabel(description, q);

    QIcon icon;
    switch (state) {
        case Tp::Contact::PresenceStateYes:
            icon = QIcon::fromTheme(QStringLiteral("task-complete"));
            break;
        case Tp::Contact::PresenceStateNo:
            icon = QIcon::fromTheme(QStringLiteral("task-reject"));
            break;
        case Tp::Contact::PresenceStateAsk:
        default:
            icon = QIcon::fromTheme(QStringLiteral("task-attempt"));
            break;
    }

    QLabel *stateLabel = new QLabel(q);
    stateLabel->setPixmap(icon.pixmap(16));

    stateLayout->addRow(descriptionLabel, stateLabel);
}

void ContactInfoDialog::Private::loadStateRows()
{
    if(stateLayout) {
        addStateRow(i18n("Contact can see when you are online:"), contact->publishState());
        addStateRow(i18n("You can see when the contact is online:"), contact->subscriptionState());
        addStateRow(i18n("Contact is blocked:"), contact->isBlocked() ? Tp::Contact::PresenceStateYes : Tp::Contact::PresenceStateNo);
    }
}

ContactInfoDialog::ContactInfoDialog(const Tp::AccountPtr &account, const Tp::ContactPtr &contact, QWidget *parent)
    : QDialog(parent)
    , d(new Private(this))
{
#if 0   // Editing contacts is not yet supported in TpQt
    /* Whether contact is the user himself */
    d->editable = (contact == account->connection()->selfContact());
#endif
    d->editable = false;
    d->account = account;
    d->contact = KTp::ContactPtr::qObjectCast(contact);

    d->buttonBox = new QDialogButtonBox(this);


    if (d->editable) {
        d->buttonBox->setStandardButtons(QDialogButtonBox::Save | QDialogButtonBox::Close);
    } else {
        d->buttonBox->setStandardButtons(QDialogButtonBox::Close);
    }

    connect(d->buttonBox, &QDialogButtonBox::clicked, this, &ContactInfoDialog::slotButtonClicked);

    setMaximumSize(sizeHint());

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(30);

    /* Title - presence icon, alias, id */
    KTitleWidget *titleWidget = new KTitleWidget(this);
    KTp::Presence presence(contact->presence());
    titleWidget->setPixmap(presence.icon().pixmap(32, 32), KTitleWidget::ImageLeft);
    titleWidget->setText(contact->alias());
    titleWidget->setComment(contact->id());
    layout->addWidget(titleWidget);

    /* 1st column: avatar; 2nd column: details */
    d->columnsLayout = new QHBoxLayout();
    d->columnsLayout->setSpacing(30);
    layout->addLayout(d->columnsLayout);

    /* Make sure the contact has all neccessary features ready */
    Tp::PendingContacts *op = contact->manager()->upgradeContacts(
            QList<Tp::ContactPtr>() << contact,
            Tp::Features() << Tp::Contact::FeatureAvatarData
                           << Tp::Contact::FeatureInfo);
    connect(op, SIGNAL(finished(Tp::PendingOperation*)), SLOT(onContactUpgraded(Tp::PendingOperation*)));

    /* State Info - there is no point showing this information when it's about ourselves */
    if (!d->editable) {
        d->stateLayout = new QFormLayout();
        d->stateLayout->setSpacing(10);
        layout->addLayout(d->stateLayout);

        // Fetch roster feature, if it is supported, but not loaded
        Tp::ConnectionPtr conn = contact->manager()->connection();
        if(!conn->actualFeatures().contains(Tp::Connection::FeatureRoster) && !conn->missingFeatures().contains(Tp::Connection::FeatureRoster)) {
            Tp::PendingReady *pr = conn->becomeReady(Tp::Features() << Tp::Connection::FeatureRoster);

            connect(pr, SIGNAL(finished(Tp::PendingOperation*)),
                    SLOT(onFeatureRosterReady(Tp::PendingOperation*)));
        } else {
            d->loadStateRows();
        }
    }

    layout->addWidget(d->buttonBox);
}

ContactInfoDialog::~ContactInfoDialog()
{
    delete d;
}

void ContactInfoDialog::slotButtonClicked(QAbstractButton *button)
{
    if (button == d->buttonBox->button(QDialogButtonBox::Save)) {
        if (d->avatarChanged) {
            Tp::Avatar avatar;
            if (!d->newAvatarFile.isEmpty()) {
                QFile file(d->newAvatarFile);
                file.open(QIODevice::ReadOnly);

                QFileInfo fi(file);

                avatar.avatarData = file.readAll();
                file.seek(0); // reset before passing to KMimeType

                QMimeDatabase db;
                avatar.MIMEType = db.mimeTypeForFileNameAndData(d->newAvatarFile, &file).name();
            }

            d->account->setAvatar(avatar);
        }

        if (d->infoDataChanged) {
            Tp::ContactInfoFieldList fieldList;

            for (InfoRowIndex index = (InfoRowIndex) 0; index < _InfoRowCount; index = (InfoRowIndex) (index + 1)) {
                InfoRow *row = &InfoRows[index];

                Tp::ContactInfoField field;
                field.fieldName = row->fieldName;

                if (index == Birthday) {
                    KDateComboBox *combo = qobject_cast<KDateComboBox*>(d->infoValueWidgets.value(index));
                    field.fieldValue << combo->date().toString();
                } else {
                    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(d->infoValueWidgets.value(index));
                    field.fieldValue << lineEdit->text();
                }

                fieldList << field;
            }

#if 0   // This method does not exist in TpQt (yet)
            d->account->connection()->setContactInfo(fieldList);
#endif
        }

        accept();
        return;
    }
}


} /* namespace KTp */

#include "moc_contact-info-dialog.cpp"
