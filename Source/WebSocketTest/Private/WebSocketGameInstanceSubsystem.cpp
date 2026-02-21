#include "WebSocketGameInstanceSubsystem.h"
#include "WebSocketLog.h"
#include "WebSocketsModule.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogWebSocket);

void UWebSocketGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogWebSocket, Log, TEXT("WebSocketGameInstanceSubsystem initialized"));
}

void UWebSocketGameInstanceSubsystem::Deinitialize()
{
	Disconnect();
	Super::Deinitialize();
}

void UWebSocketGameInstanceSubsystem::Connect(const FString& ServerURL)
{
	// Si déjà connecté, on déconnecte d'abord
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		Disconnect();
	}

	// S'assurer que le module WebSockets est chargé
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	// Créer la WebSocket
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerURL);

	// Binder les callbacks
	WebSocket->OnConnected().AddUObject(this, &UWebSocketGameInstanceSubsystem::OnConnectedCallback);
	WebSocket->OnConnectionError().AddUObject(this, &UWebSocketGameInstanceSubsystem::OnConnectionErrorCallback);
	WebSocket->OnClosed().AddUObject(this, &UWebSocketGameInstanceSubsystem::OnClosedCallback);
	WebSocket->OnMessage().AddUObject(this, &UWebSocketGameInstanceSubsystem::OnMessageCallback);
	WebSocket->OnRawMessage().AddUObject(this, &UWebSocketGameInstanceSubsystem::OnRawMessageCallback);

	// Lancer la connexion
	WebSocket->Connect();

	UE_LOG(LogWebSocket, Log, TEXT("Connecting to WebSocket server: %s"), *ServerURL);
}

void UWebSocketGameInstanceSubsystem::Disconnect()
{
	if (WebSocket.IsValid())
	{
		if (WebSocket->IsConnected())
		{
			WebSocket->Close();
		}
		WebSocket.Reset();
		UE_LOG(LogWebSocket, Log, TEXT("WebSocket disconnected"));
	}
}

void UWebSocketGameInstanceSubsystem::SendMessage(const FString& Message)
{
	if (WebSocket.IsValid() && WebSocket->IsConnected())
	{
		WebSocket->Send(Message);
		UE_LOG(LogWebSocket, Verbose, TEXT("Message sent: %s"), *Message);
	}
	else
	{
		UE_LOG(LogWebSocket, Warning, TEXT("Cannot send message: WebSocket not connected"));
	}
}

bool UWebSocketGameInstanceSubsystem::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

void UWebSocketGameInstanceSubsystem::SendReady(int32 PlayerId)
{
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	JsonObject->SetStringField("type", "ready");
	JsonObject->SetNumberField("player", PlayerId);

	FString OutputString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

	SendMessage(OutputString);
}

void UWebSocketGameInstanceSubsystem::OnConnectedCallback()
{
	UE_LOG(LogWebSocket, Log, TEXT("WebSocket connected!"));
	OnConnected.Broadcast();
}

void UWebSocketGameInstanceSubsystem::OnConnectionErrorCallback(const FString& Error)
{
	UE_LOG(LogWebSocket, Error, TEXT("WebSocket connection error: %s"), *Error);
	OnError.Broadcast(Error);
}

void UWebSocketGameInstanceSubsystem::OnClosedCallback(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogWebSocket, Log, TEXT("WebSocket closed: %s (Status: %d, Clean: %d)"), *Reason, StatusCode, bWasClean);
	OnClosed.Broadcast();
}

void UWebSocketGameInstanceSubsystem::OnMessageCallback(const FString& Message)
{
	UE_LOG(LogWebSocket, Verbose, TEXT("WebSocket message received (text): %s"), *Message);
	OnMessageReceived.Broadcast(Message);
}

void UWebSocketGameInstanceSubsystem::OnRawMessageCallback(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
{
	// Pour les messages binaires (si besoin)
	UE_LOG(LogWebSocket, VeryVerbose, TEXT("Raw message received, size: %d, remaining: %d"), Size, BytesRemaining);
}

FString UWebSocketGameInstanceSubsystem::GetUniqueSocketId() const
{
	// Utile si vous voulez gérer plusieurs connexions
	return FString::Printf(TEXT("WebSocket_%d"), FMath::Rand());
}