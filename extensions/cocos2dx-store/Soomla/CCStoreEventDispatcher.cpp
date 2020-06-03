

#include "CCStoreEventDispatcher.h"
#include "CCSoomlaEventDispatcher.h"
#include "CCError.h"
#include "CCStoreInfo.h"
#include "CCSoomlaUtils.h"
#include "CCPurchaseWithMarket.h"
#include "CCNdkBridge.h"
#include "CCStoreInventory.h"

namespace soomla {

    USING_NS_CC;

    static CCStoreEventDispatcher *s_SharedInstance = NULL;

    CCStoreEventDispatcher *CCStoreEventDispatcher::getInstance() {
        if (!s_SharedInstance) {
            s_SharedInstance = new CCStoreEventDispatcher();
            s_SharedInstance->init();
        }

        return s_SharedInstance;
    }

    bool CCStoreEventDispatcher::init() {

        if (!CCAbstractAggregatedEventHandler::init()) {
            return false;
        }

        CCSoomlaEventDispatcher *eventDispatcher = CCSoomlaEventDispatcher::getInstance();

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_BILLING_NOT_SUPPORTED,
                [this](__Dictionary *parameters) {
                    this->onBillingNotSupported();
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_BILLING_SUPPORTED,
                [this](__Dictionary *parameters) {
                    this->onBillingSupported();
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_CURRENCY_BALANCE_CHANGED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    __Integer *balance = (__Integer *)(parameters->objectForKey("balance"));
                    __Integer *amountAdded = (__Integer *)(parameters->objectForKey("amountAdded"));
                    CCError *error = NULL;
                    CCVirtualCurrency *virtualCurrency =
                            dynamic_cast<CCVirtualCurrency *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_CURRENCY_BALANCE_CHANGED, error);
                        return;
                    }
                    CC_ASSERT(virtualCurrency);
                    this->onCurrencyBalanceChanged(virtualCurrency, balance->getValue(), amountAdded->getValue());
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_GOOD_BALANCE_CHANGED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    __Integer *balance = (__Integer *)(parameters->objectForKey("balance"));
                    __Integer *amountAdded = (__Integer *)(parameters->objectForKey("amountAdded"));
                    CCError *error = NULL;
                    CCVirtualGood *virtualGood =
                            dynamic_cast<CCVirtualGood *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_GOOD_BALANCE_CHANGED, error);
                        return;
                    }
                    CC_ASSERT(virtualGood);
                    this->onGoodBalanceChanged(virtualGood, balance->getValue(), amountAdded->getValue());
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_GOOD_EQUIPPED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCEquippableVG *equippableVG =
                            dynamic_cast<CCEquippableVG *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_GOOD_EQUIPPED, error);
                        return;
                    }
                    CC_ASSERT(equippableVG);
                    this->onGoodEquipped(equippableVG);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_GOOD_UNEQUIPPED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCEquippableVG *equippableVG =
                            dynamic_cast<CCEquippableVG *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_GOOD_UNEQUIPPED, error);
                        return;
                    }
                    CC_ASSERT(equippableVG);
                    this->onGoodUnEquipped(equippableVG);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_GOOD_UPGRADE,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    __String *vguItemId = (__String *)(parameters->objectForKey("vguItemId"));

                    CCError *error;

                    error = NULL;
                    CCVirtualGood *virtualGood =
                            dynamic_cast<CCVirtualGood *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_GOOD_UPGRADE, error);
                        return;
                    }
                    CC_ASSERT(virtualGood);

                    error = NULL;
                    CCUpgradeVG *upgradeVG =
                            dynamic_cast<CCUpgradeVG *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(vguItemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_GOOD_UPGRADE, error);
                        return;
                    }
                    CC_ASSERT(upgradeVG);
                    this->onGoodUpgrade(virtualGood, upgradeVG);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_ITEM_PURCHASED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    __String *payload = (__String *)(parameters->objectForKey("payload"));
                    CCError *error = NULL;
                    CCPurchasableVirtualItem *purchasableVirtualItem =
                            dynamic_cast<CCPurchasableVirtualItem *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_ITEM_PURCHASED, error);
                        return;
                    }
                    CC_ASSERT(purchasableVirtualItem);
                    this->onItemPurchased(purchasableVirtualItem, payload);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_ITEM_PURCHASE_STARTED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCPurchasableVirtualItem *purchasableVirtualItem = dynamic_cast<CCPurchasableVirtualItem *>(
                            CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_ITEM_PURCHASE_STARTED, error);
                        return;
                    }
                    CC_ASSERT(purchasableVirtualItem);
                    this->onItemPurchaseStarted(purchasableVirtualItem);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_MARKET_PURCHASE_CANCELED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCPurchasableVirtualItem *purchasableVirtualItem =
                            dynamic_cast<CCPurchasableVirtualItem *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_MARKET_PURCHASE_CANCELED, error);
                        return;
                    }
                    CC_ASSERT(purchasableVirtualItem);
                    this->onMarketPurchaseCancelled(purchasableVirtualItem);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_MARKET_PURCHASE,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCPurchasableVirtualItem *purchasableVirtualItem =
                            dynamic_cast<CCPurchasableVirtualItem *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_MARKET_PURCHASE, error);
                        return;
                    }
                    CC_ASSERT(purchasableVirtualItem);
                    __String *token = (__String *)(parameters->objectForKey("token"));
                    __String *payload = (__String *)(parameters->objectForKey("payload"));
                    __SetIterator i;
                    this->onMarketPurchase(purchasableVirtualItem, token, payload);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_MARKET_PURCHASE_STARTED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCPurchasableVirtualItem *purchasableVirtualItem = dynamic_cast<CCPurchasableVirtualItem *>(
                            CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_MARKET_PURCHASE_STARTED, error);
                        return;
                    }
                    CC_ASSERT(purchasableVirtualItem);
                    this->onMarketPurchaseStarted(purchasableVirtualItem);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_MARKET_PURCHASE_STARTED,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCPurchasableVirtualItem *purchasableVirtualItem =
                            dynamic_cast<CCPurchasableVirtualItem *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_MARKET_PURCHASE_STARTED, error);
                        return;
                    }
                    CC_ASSERT(purchasableVirtualItem);
                    this->onMarketPurchaseStarted(purchasableVirtualItem);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_MARKET_ITEMS_REFRESHED,
                [this](__Dictionary *parameters) {
                    __Array *marketItemDicts = (__Array *)(parameters->objectForKey("marketItems"));
                    __Array *marketItems = __Array::create();

                    CCError *error = NULL;
                    __Dictionary *marketItem = NULL;
                    for (unsigned int i = 0; i < marketItemDicts->count(); i++) {
                        marketItem = dynamic_cast<__Dictionary *>(marketItemDicts->getObjectAtIndex(i));
                        CC_ASSERT(marketItem);
                        __String *productId = dynamic_cast<__String *>(marketItem->objectForKey("productId"));
                        __String *marketPrice = dynamic_cast<__String *>(marketItem->objectForKey("marketPrice"));
                        __String *marketTitle = dynamic_cast<__String *>(marketItem->objectForKey("marketTitle"));
                        __String *marketDescription = dynamic_cast<__String *>(marketItem->objectForKey("marketDesc"));
                        __String *marketCurrencyCode = dynamic_cast<__String *>(marketItem->objectForKey("marketCurrencyCode"));
                        __Integer *marketPriceMicros = dynamic_cast<__Integer *>(marketItem->objectForKey("marketPriceMicros"));

                        CCPurchasableVirtualItem *pvi = CCStoreInfo::sharedStoreInfo()->getPurchasableItemWithProductId(
                                productId->getCString(), &error);
                        if (error) {
                            CCSoomlaUtils::logException(CCStoreConsts::EVENT_MARKET_ITEMS_REFRESHED, error);
                            return;
                        }
                        CC_ASSERT(pvi);

                        CCPurchaseWithMarket *purchaseWithMarket = dynamic_cast<CCPurchaseWithMarket *>(pvi->getPurchaseType());
                        CC_ASSERT(purchaseWithMarket);
                        CCMarketItem *mi = purchaseWithMarket->getMarketItem();
                        mi->setMarketPriceAndCurrency(marketPrice);
                        mi->setMarketTitle(marketTitle);
                        mi->setMarketDescription(marketDescription);
                        mi->setMarketCurrencyCode(marketCurrencyCode);
                        mi->setMarketPriceMicros(marketPriceMicros);
                        pvi->save();

                        marketItems->addObject(purchaseWithMarket);
                    }

                    this->onMarketItemsRefreshed(marketItems);
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_MARKET_ITEMS_REFRESH_STARTED,
                [this](__Dictionary *parameters) {
                    this->onMarketItemsRefreshStarted();
                });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_MARKET_PURCHASE_VERIFICATION,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCPurchasableVirtualItem *purchasableVirtualItem =
                            dynamic_cast<CCPurchasableVirtualItem *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_MARKET_PURCHASE_VERIFICATION, error);
                        return;
                    }
                    CC_ASSERT(purchasableVirtualItem);
                    this->onMarketPurchaseVerification(purchasableVirtualItem);
               });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_RESTORE_TRANSACTION_FINISHED,
                [this](__Dictionary *parameters) {
                    __Bool *success = (__Bool *)(parameters->objectForKey("success"));
                    this->onRestoreTransactionsFinished(success->getValue());
               });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_RESTORE_TRANSACTION_STARTED,
                [this](__Dictionary *parameters) {
                    this->onRestoreTransactionsStarted();
               });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_UNEXPECTED_ERROR_IN_STORE,
                [this](__Dictionary *parameters) {
                    __String *errorMessage = (__String *)(parameters->objectForKey("errorMessage"));
                    this->onUnexpectedErrorInStore(errorMessage);
               });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_STORE_CONTROLLER_INITIALIZED,
                [this](__Dictionary *parameters) {
                    this->onSoomlaStoreInitialized();
               });

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_MARKET_REFUND,
                [this](__Dictionary *parameters) {
                    __String *itemId = (__String *)(parameters->objectForKey("itemId"));
                    CCError *error = NULL;
                    CCPurchasableVirtualItem *purchasableVirtualItem =
                            dynamic_cast<CCPurchasableVirtualItem *>(CCStoreInfo::sharedStoreInfo()->getItemByItemId(itemId->getCString(), &error));
                    if (error) {
                        CCSoomlaUtils::logException(CCStoreConsts::EVENT_MARKET_REFUND, error);
                        return;
                    }
                    CC_ASSERT(purchasableVirtualItem);
                    this->onMarketRefund(purchasableVirtualItem);
               });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_IAB_SERVICE_STARTED,
                [this](__Dictionary *parameters) {
                    this->onIabServiceStarted();
               });

        eventDispatcher->registerEventHandler(CCStoreConsts::EVENT_IAB_SERVICE_STOPPED,
                [this](__Dictionary *parameters) {
                    this->onIabServiceStopped();
               });
#endif
        return true;
    }

    void CCStoreEventDispatcher::onBillingNotSupported() {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onBillingNotSupported();
        }
    }

    void CCStoreEventDispatcher::onBillingSupported() {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onBillingSupported();
        }
    }

    void CCStoreEventDispatcher::onCurrencyBalanceChanged(CCVirtualCurrency *virtualCurrency, int balance, int amountAdded) {
        CCStoreInventory::sharedStoreInventory()->refreshOnCurrencyBalanceChanged(virtualCurrency, balance, amountAdded);
        
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onCurrencyBalanceChanged(virtualCurrency, balance, amountAdded);
        }
    }

    void CCStoreEventDispatcher::onGoodBalanceChanged(CCVirtualGood *virtualGood, int balance, int amountAdded) {
        CCStoreInventory::sharedStoreInventory()->refreshOnGoodBalanceChanged(virtualGood, balance, amountAdded);
        
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onGoodBalanceChanged(virtualGood, balance, amountAdded);
        }
    }

    void CCStoreEventDispatcher::onGoodEquipped(CCEquippableVG *equippableVG) {
        CCStoreInventory::sharedStoreInventory()->refreshOnGoodEquipped(equippableVG);
        
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onGoodEquipped(equippableVG);
        }
    }

    void CCStoreEventDispatcher::onGoodUnEquipped(CCEquippableVG *equippableVG) {
        CCStoreInventory::sharedStoreInventory()->refreshOnGoodUnEquipped(equippableVG);
        
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onGoodUnEquipped(equippableVG);
        }
    }

    void CCStoreEventDispatcher::onGoodUpgrade(CCVirtualGood *virtualGood, CCUpgradeVG *upgradeVG) {
        CCStoreInventory::sharedStoreInventory()->refreshOnGoodUpgrade(virtualGood, upgradeVG);
        
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onGoodUpgrade(virtualGood, upgradeVG);
        }
    }

    void CCStoreEventDispatcher::onItemPurchased(CCPurchasableVirtualItem *purchasableVirtualItem, cocos2d::__String *payload) {
        onItemPurchased(purchasableVirtualItem, payload, false);
    }

    void CCStoreEventDispatcher::onItemPurchased(CCPurchasableVirtualItem *purchasableVirtualItem, cocos2d::__String *payload, bool alsoPush) {
        if (payload == NULL) {
            payload = __String::create("");
        }
        
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onItemPurchased(purchasableVirtualItem, payload);
        }

        if (alsoPush) {
            #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            
            __Dictionary *params = __Dictionary::create();
            params->setObject(__String::create("CCStoreEventDispatcher::pushOnItemPurchased"), "method");
            params->setObject(purchasableVirtualItem->getItemId(), "itemId");
            params->setObject(payload, "payload");
            CCNdkBridge::callNative (params, NULL);
            
            #endif
        }
    }

    void CCStoreEventDispatcher::onItemPurchaseStarted(CCPurchasableVirtualItem *purchasableVirtualItem) {
        onItemPurchaseStarted(purchasableVirtualItem, false);
    }

    void CCStoreEventDispatcher::onItemPurchaseStarted(CCPurchasableVirtualItem *purchasableVirtualItem, bool alsoPush) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onItemPurchaseStarted(purchasableVirtualItem);
        }

        if (alsoPush) {
        #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            
            __Dictionary *params = __Dictionary::create();
            params->setObject(__String::create("CCStoreEventDispatcher::pushOnItemPurchaseStarted"), "method");
            params->setObject(purchasableVirtualItem->getItemId(), "itemId");
            CCNdkBridge::callNative (params, NULL);
            
        #endif
        }
    }

    void CCStoreEventDispatcher::onMarketPurchaseCancelled(CCPurchasableVirtualItem *purchasableVirtualItem) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onMarketPurchaseCancelled(purchasableVirtualItem);
        }
    }

    void CCStoreEventDispatcher::onMarketPurchase(CCPurchasableVirtualItem *purchasableVirtualItem, cocos2d::__String *token, cocos2d::__String *payload) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onMarketPurchase(purchasableVirtualItem, token, payload);
        }
    }

    void CCStoreEventDispatcher::onMarketPurchaseStarted(CCPurchasableVirtualItem *purchasableVirtualItem) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onMarketPurchaseStarted(purchasableVirtualItem);
        }
    }

    void CCStoreEventDispatcher::onMarketPurchaseVerification(CCPurchasableVirtualItem *purchasableVirtualItem) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onMarketPurchaseVerification(purchasableVirtualItem);
        }
    }

    void CCStoreEventDispatcher::onRestoreTransactionsFinished(bool success) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onRestoreTransactionsFinished(success);
        }
    }

    void CCStoreEventDispatcher::onRestoreTransactionsStarted() {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onRestoreTransactionsStarted();
        }
    }

    void CCStoreEventDispatcher::onUnexpectedErrorInStore(cocos2d::__String *errorMessage) {
        onUnexpectedErrorInStore(errorMessage, false);
    }

    void CCStoreEventDispatcher::onUnexpectedErrorInStore(cocos2d::__String *errorMessage, bool alsoPush) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onUnexpectedErrorInStore(errorMessage);
        }

        if (alsoPush) {
            #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
                        
            __Dictionary *params = __Dictionary::create();
            params->setObject(__String::create("CCStoreEventDispatcher::pushOnUnexpectedErrorInStore"), "method");
            params->setObject(errorMessage, "errorMessage");
            CCNdkBridge::callNative (params, NULL);
            
            #endif
        }
    }

    void CCStoreEventDispatcher::onSoomlaStoreInitialized() {
        onSoomlaStoreInitialized(false);
    }

    void CCStoreEventDispatcher::onSoomlaStoreInitialized(bool alsoPush) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onSoomlaStoreInitialized();
        }

        CCStoreInventory::sharedStoreInventory()->refreshLocalInventory();

        if (alsoPush) {
        #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
            
            __Dictionary *params = __Dictionary::create();
            params->setObject(__String::create("CCStoreEventDispatcher::pushOnSoomlaStoreInitialized"), "method");
            CCNdkBridge::callNative (params, NULL);
            
        #endif
        }
    }

    void CCStoreEventDispatcher::onMarketItemsRefreshed(cocos2d::__Array *virtualItems) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onMarketItemsRefreshed(virtualItems);
        }
    }

    void CCStoreEventDispatcher::onMarketItemsRefreshStarted() {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onMarketItemsRefreshStarted();
        }
    }

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
    void CCStoreEventDispatcher::onMarketRefund(CCPurchasableVirtualItem *purchasableVirtualItem) {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onMarketRefund(purchasableVirtualItem);
        }
    }

    void CCStoreEventDispatcher::onIabServiceStarted() {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onIabServiceStarted();
        }
    }

    void CCStoreEventDispatcher::onIabServiceStopped() {
        FOR_EACH_EVENT_HANDLER(CCStoreEventHandler)
            eventHandler->onIabServiceStopped();
        }
    }
#endif

}
