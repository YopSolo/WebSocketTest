#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WebSocketGameInstanceSubsystem.generated.h"

// Delegates pour exposer les événements aux Blueprints
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketConnectedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketMessageSignature, const FString&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWebSocketErrorSignature, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWebSocketClosedSignature);

UCLASS()
class WEBSOCKETTEST_API UWebSocketGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Initialisation et nettoyage
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Fonctions appelables depuis les Blueprints
	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void Connect(const FString& ServerURL);

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "WebSocket")
	void SendMessage(const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "WebSocket", BlueprintPure)
	bool IsConnected() const;

	// Delegates exposés aux Blueprints
	UPROPERTY(BlueprintAssignable, Category = "WebSocket")
	FOnWebSocketConnectedSignature OnConnected;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket")
	FOnWebSocketMessageSignature OnMessageReceived;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket")
	FOnWebSocketErrorSignature OnError;

	UPROPERTY(BlueprintAssignable, Category = "WebSocket")
	FOnWebSocketClosedSignature OnClosed;

	// -- 
	UFUNCTION(BlueprintCallable, Category = "GameMessage")
	void SendReady(int32 PlayerId);


private:
	TSharedPtr<IWebSocket> WebSocket;

	// Callbacks internes
	void OnConnectedCallback();
	void OnConnectionErrorCallback(const FString& Error);
	void OnClosedCallback(int32 StatusCode, const FString& Reason, bool bWasClean);
	void OnMessageCallback(const FString& Message);
	void OnRawMessageCallback(const void* Data, SIZE_T Size, SIZE_T BytesRemaining);

	// Utilitaires
	FString GetUniqueSocketId() const;
};