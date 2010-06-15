/***************************************************************************
                          kmymoneypricedlg.cpp
                             -------------------
    begin                : Wed Nov 24 2004
    copyright            : (C) 2000-2004 by Michael Edwardes
    email                : mte@users.sourceforge.net
                           Javier Campos Morales <javi_c@users.sourceforge.net>
                           Felix Rodriguez <frodriguez@users.sourceforge.net>
                           John C <thetacoturtle@users.sourceforge.net>
                           Thomas Baumgart <ipwizard@users.sourceforge.net>
                           Kevin Tambascio <ktambascio@users.sourceforge.net>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kmymoneypricedlg.h"

// ----------------------------------------------------------------------------
// QT Includes

#include <QCheckBox>

// ----------------------------------------------------------------------------
// KDE Includes

#include <kpushbutton.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kmessagebox.h>

// ----------------------------------------------------------------------------
// Project Includes

#include "kupdatestockpricedlg.h"
#include "kcurrencycalculator.h"
#include <mymoneyprice.h>
#include "kequitypriceupdatedlg.h"
#include <kmymoneycurrencyselector.h>
#include <mymoneyfile.h>
#include "kmymoneyglobalsettings.h"

KMyMoneyPriceDlg::KMyMoneyPriceDlg(QWidget* parent) :
    KMyMoneyPriceDlgDecl(parent)
{
  setButtons(KDialog::Ok);
  setButtonsOrientation(Qt::Horizontal);
  setMainWidget(m_layoutWidget);

  // create the searchline widget
  // and insert it into the existing layout
  m_searchWidget = new KTreeWidgetSearchLineWidget(this, m_priceList);
  m_searchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
  m_listLayout->insertWidget(0, m_searchWidget);

  m_priceList->header()->setStretchLastSection(true);
  m_priceList->setContextMenuPolicy(Qt::CustomContextMenu);

  KGuiItem removeButtonItem(i18n("&Delete"),
                            KIcon("edit-delete"),
                            i18n("Delete this entry"),
                            i18n("Remove this price item from the file"));
  m_deleteButton->setGuiItem(removeButtonItem);

  KGuiItem newButtonItem(i18nc("New price entry", "&New"),
                         KIcon("document-new"),
                         i18n("Add a new entry"),
                         i18n("Create a new price entry."));
  m_newButton->setGuiItem(newButtonItem);

  KGuiItem editButtonItem(i18n("&Edit"),
                          KIcon("document-edit"),
                          i18n("Modify the selected entry"),
                          i18n("Change the details of selected price information."));
  m_editButton->setGuiItem(editButtonItem);

  m_onlineQuoteButton->setIcon(KIcon("investment-update-online"));

  connect(m_editButton, SIGNAL(clicked()), this, SLOT(slotEditPrice()));
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(slotDeletePrice()));
  connect(m_newButton, SIGNAL(clicked()), this, SLOT(slotNewPrice()));
  connect(m_priceList, SIGNAL(itemSelectionChanged()), this, SLOT(slotSelectPrice()));
  connect(m_onlineQuoteButton, SIGNAL(clicked()), this, SLOT(slotOnlinePriceUpdate()));
  connect(m_priceList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotOpenContextMenu(const QPoint&)));

  connect(m_showAllPrices, SIGNAL(toggled(bool)), this, SLOT(slotLoadWidgets()));
  connect(MyMoneyFile::instance(), SIGNAL(dataChanged()), this, SLOT(slotLoadWidgets()));

  //get the price precision
  KSharedConfigPtr kconfig = KGlobal::config();
  KConfigGroup grp =  kconfig->group("General Options");
  m_pricePrecision = grp.readEntry("PricePrecision", 4);

  slotLoadWidgets();
}

KMyMoneyPriceDlg::~KMyMoneyPriceDlg()
{
}

void KMyMoneyPriceDlg::slotLoadWidgets(void)
{
  m_priceList->clear();
  m_priceList->setSortingEnabled(false);

  MyMoneyPriceList list = MyMoneyFile::instance()->priceList();
  MyMoneyPriceList::ConstIterator it_allPrices;
  for (it_allPrices = list.constBegin(); it_allPrices != list.constEnd(); ++it_allPrices) {
    MyMoneyPriceEntries::ConstIterator it_priceItem;
    if (m_showAllPrices->isChecked()) {
      for (it_priceItem = (*it_allPrices).constBegin(); it_priceItem != (*it_allPrices).constEnd(); ++it_priceItem) {
        loadPriceItem(*it_priceItem);
      }
    } else {
      //if it doesn't show all prices, it only shows the most recent occurrence for each price
      if ((*it_allPrices).count() > 0) {
        //the prices for each currency are ordered by date in ascending order
        //it gets the last item of the item, which is supposed to be the most recent price
        it_priceItem = (*it_allPrices).constEnd();
        --it_priceItem;
        loadPriceItem(*it_priceItem);
      }
    }
  }
  m_priceList->setSortingEnabled(true);
  m_priceList->sortByColumn(ePriceCommodity);
}

QTreeWidgetItem* KMyMoneyPriceDlg::loadPriceItem(const MyMoneyPrice& basePrice)
{
  MyMoneySecurity from, to;
  MyMoneyPrice price = MyMoneyPrice(basePrice);

  QTreeWidgetItem* priceTreeItem = new QTreeWidgetItem(m_priceList);

  if (!price.isValid())
    price = MyMoneyFile::instance()->price(price.from(), price.to(), price.date());

  if (price.isValid()) {
    QString priceBase = price.to();
    from = MyMoneyFile::instance()->security(price.from());
    to = MyMoneyFile::instance()->security(price.to());
    if (!to.isCurrency()) {
      from = MyMoneyFile::instance()->security(price.to());
      to = MyMoneyFile::instance()->security(price.from());
      priceBase = price.from();
    }

    priceTreeItem->setData(ePriceCommodity, Qt::UserRole, QVariant::fromValue(price));
    priceTreeItem->setText(ePriceCommodity, (from.isCurrency()) ? from.id() : from.tradingSymbol());
    priceTreeItem->setText(ePriceCurrency, to.id());
    priceTreeItem->setText(ePriceDate, KGlobal::locale()->formatDate(price.date(), KLocale::ShortDate));
    priceTreeItem->setText(ePricePrice, price.rate(priceBase).formatMoney("", m_pricePrecision));
    priceTreeItem->setText(ePriceSource, price.source());
  }
  return priceTreeItem;
}

void KMyMoneyPriceDlg::slotSelectPrice()
{
  QTreeWidgetItem* item = 0;
  if(m_priceList->selectedItems().count() > 0) {
    item = m_priceList->selectedItems().at(0);
  }
  m_currentItem = item;
  m_editButton->setEnabled(item != 0);
  m_deleteButton->setEnabled(item != 0);

  //if one of the selected entries is a default, then deleting is disabled
  QList<QTreeWidgetItem*> itemsList = m_priceList->selectedItems();
  QList<QTreeWidgetItem*>::const_iterator item_it;
  bool deleteEnabled = true;
  for(item_it = itemsList.constBegin(); item_it != itemsList.constEnd(); ++item_it) {
    MyMoneyPrice price = (*item_it)->data(0, Qt::UserRole).value<MyMoneyPrice>();
    if (price.source() == "KMyMoney")
      deleteEnabled = false;
  }
  m_deleteButton->setEnabled(deleteEnabled);

  // Modification of automatically added entries is not allowed
  if (item) {
    MyMoneyPrice price = item->data(0, Qt::UserRole).value<MyMoneyPrice>();
    if (price.source() == "KMyMoney")
      m_editButton->setEnabled(false);
    emit selectObject(price);
  }
}

void KMyMoneyPriceDlg::slotNewPrice(void)
{
  QPointer<KUpdateStockPriceDlg> dlg = new KUpdateStockPriceDlg(this);
  try {
    QTreeWidgetItem* item = m_priceList->currentItem();
    if (item) {
      MyMoneySecurity security;
      security = MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().from());
      dlg->m_security->setSecurity(security);
      security = MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().to());
      dlg->m_currency->setSecurity(security);
    }

    if (dlg->exec()) {
      MyMoneyPrice price(dlg->m_security->security().id(), dlg->m_currency->security().id(), dlg->date(), MyMoneyMoney(1, 1));
      QTreeWidgetItem* p = loadPriceItem(price);
      m_priceList->setCurrentItem(p, true);
      // If the user cancels the following operation, we delete the new item
      // and re-select any previously selected one
      if (slotEditPrice() == Rejected) {
        delete p;
        if (item)
          m_priceList->setCurrentItem(item, true);
      }
    }
  } catch (...) {
    delete dlg;
    throw;
  }
  delete dlg;
}

int KMyMoneyPriceDlg::slotEditPrice(void)
{
  int rc = Rejected;
  QTreeWidgetItem* item = m_priceList->currentItem();
  if (item) {
    MyMoneySecurity from(MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().from()));
    MyMoneySecurity to(MyMoneyFile::instance()->security(item->data(0, Qt::UserRole).value<MyMoneyPrice>().to()));
    signed64 fract = MyMoneyMoney::precToDenom(KMyMoneyGlobalSettings::pricePrecision());

    QPointer<KCurrencyCalculator> calc =
      new KCurrencyCalculator(from,
                              to,
                              MyMoneyMoney(1, 1),
                              item->data(0, Qt::UserRole).value<MyMoneyPrice>().rate(to.id()),
                              item->data(0, Qt::UserRole).value<MyMoneyPrice>().date(),
                              fract,
                              this);
    calc->setupPriceEditor();

    rc = calc->exec();
    delete calc;
  }
  return rc;
}


void KMyMoneyPriceDlg::slotDeletePrice(void)
{
  QList<QTreeWidgetItem*> listItems = m_priceList->selectedItems();
  if (listItems.count() > 0) {
    if (KMessageBox::questionYesNo(this, i18np("Do you really want to delete the selected price entry?", "Do you really want to delete the selected price entries?", listItems.count() ), i18n("Delete price information"), KStandardGuiItem::yes(), KStandardGuiItem::no(), "DeletePrice") == KMessageBox::Yes) {
      MyMoneyFileTransaction ft;
      try {
        QList<QTreeWidgetItem*>::const_iterator price_it;
        for(price_it = listItems.constBegin(); price_it != listItems.constEnd(); ++price_it) {
          MyMoneyFile::instance()->removePrice((*price_it)->data(0, Qt::UserRole).value<MyMoneyPrice>());
        }
        ft.commit();
      } catch (MyMoneyException *e) {
        qDebug("Cannot delete price");
        delete e;
      }
    }
  }
}

void KMyMoneyPriceDlg::slotOnlinePriceUpdate(void)
{
  QTreeWidgetItem* item = m_priceList->currentItem();
  if (item) {
    QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this, (item->text(ePriceCommodity) + ' ' + item->text(ePriceCurrency)).toUtf8());
    if (dlg->exec() == Accepted)
      dlg->storePrices();
    delete dlg;
  } else {
    QPointer<KEquityPriceUpdateDlg> dlg = new KEquityPriceUpdateDlg(this);
    if (dlg->exec() == Accepted)
      dlg->storePrices();
    delete dlg;
  }
}

void KMyMoneyPriceDlg::slotOpenContextMenu(const QPoint& p)
{
  QTreeWidgetItem* item = m_priceList->itemAt(p);
  if (item)
    emit openContextMenu(item->data(0, Qt::UserRole).value<MyMoneyPrice>());
}

#include "kmymoneypricedlg.moc"
