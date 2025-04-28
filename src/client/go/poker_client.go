package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"net"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"sync"
	"syscall"
)

// Card represents a playing card
type Card struct {
	Rank string `json:"rank"`
	Suit string `json:"suit"`
}

// Player represents a player in the game
type Player struct {
	ID         int    `json:"id"`
	Name       string `json:"name"`
	Chips      int    `json:"chips"`
	IsConnected bool   `json:"is_connected"`
	IsActive   bool   `json:"is_active"`
	IsReady    bool   `json:"is_ready"`
}

// GameState represents the current state of the game
type GameState struct {
	Type           string   `json:"type"`
	Stage          int      `json:"stage"`
	CurrentPlayer  int      `json:"current_player"`
	DealerPosition int      `json:"dealer_position"`
	Players        []Player `json:"players"`
	Pot            int      `json:"pot,omitempty"`
}

// PokerClient represents a client for the poker game
type PokerClient struct {
	conn           net.Conn
	playerID       int
	playerName     string
	chips          int
	privateCards   []string
	publicCards    []string
	gameState      GameState
	receivedMsg    chan []byte
	mutex          sync.Mutex
	isConnected    bool
}

// NewPokerClient creates a new poker client
func NewPokerClient(name string) *PokerClient {
	return &PokerClient{
		playerName:  name,
		receivedMsg: make(chan []byte, 10),
		isConnected: false,
	}
}

// Connect connects to the poker server
func (p *PokerClient) Connect(serverIP string, port int) error {
	addr := fmt.Sprintf("%s:%d", serverIP, port)
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return fmt.Errorf("failed to connect to server: %w", err)
	}
	
	p.conn = conn
	p.isConnected = true
	
	// Start goroutine to read messages from server
	go p.readMessages()
	
	// Register with the server
	err = p.registerPlayer()
	if err != nil {
		p.isConnected = false
		p.conn.Close()
		return fmt.Errorf("failed to register with server: %w", err)
	}
	
	fmt.Printf("Connected to server at %s\n", addr)
	return nil
}

// Disconnect disconnects from the poker server
func (p *PokerClient) Disconnect() {
	if !p.isConnected {
		return
	}
	
	p.isConnected = false
	if p.conn != nil {
		p.conn.Close()
	}
	fmt.Println("Disconnected from server")
}

// IsConnected returns true if the client is connected to the server
func (p *PokerClient) IsConnected() bool {
	return p.isConnected
}

// SetReady sets the player's ready state
func (p *PokerClient) SetReady(ready bool) error {
	msg := map[string]interface{}{
		"type":  "ready",
		"ready": ready,
	}
	
	return p.sendJSON(msg)
}

// PerformAction performs a poker action
func (p *PokerClient) PerformAction(action string, amount int) error {
	msg := map[string]interface{}{
		"type":   "action",
		"action": action,
	}
	
	if action == "raise" {
		msg["amount"] = amount
	}
	
	return p.sendJSON(msg)
}

// registerPlayer registers the player with the server
func (p *PokerClient) registerPlayer() error {
	msg := map[string]string{
		"type": "register",
		"name": p.playerName,
	}
	
	return p.sendJSON(msg)
}

// sendJSON sends a JSON message to the server
func (p *PokerClient) sendJSON(msg interface{}) error {
	data, err := json.Marshal(msg)
	if err != nil {
		return fmt.Errorf("failed to marshal JSON: %w", err)
	}
	
	p.mutex.Lock()
	defer p.mutex.Unlock()
	
	_, err = p.conn.Write(data)
	if err != nil {
		return fmt.Errorf("failed to send message: %w", err)
	}
	
	return nil
}

// readMessages reads messages from the server
func (p *PokerClient) readMessages() {
	buffer := make([]byte, 1024)
	
	for p.isConnected {
		n, err := p.conn.Read(buffer)
		if err != nil {
			fmt.Printf("Error reading from server: %v\n", err)
			p.isConnected = false
			break
		}
		
		if n > 0 {
			msg := make([]byte, n)
			copy(msg, buffer[:n])
			p.handleMessage(msg)
		}
	}
}

// handleMessage handles a message from the server
func (p *PokerClient) handleMessage(msg []byte) {
	var msgMap map[string]interface{}
	err := json.Unmarshal(msg, &msgMap)
	if err != nil {
		fmt.Printf("Error parsing message: %v\n", err)
		return
	}
	
	msgType, ok := msgMap["type"].(string)
	if !ok {
		fmt.Println("Message missing 'type' field")
		return
	}
	
	switch msgType {
	case "welcome":
		playerID, _ := msgMap["player_id"].(float64)
		p.playerID = int(playerID)
		fmt.Printf("Received welcome message. Player ID: %d\n", p.playerID)
		
	case "registered":
		playerID, _ := msgMap["player_id"].(float64)
		chips, _ := msgMap["chips"].(float64)
		p.playerID = int(playerID)
		p.chips = int(chips)
		fmt.Printf("Successfully registered. Player ID: %d, Chips: %d\n", p.playerID, p.chips)
		
	case "game_state":
		var gameState GameState
		err := json.Unmarshal(msg, &gameState)
		if err != nil {
			fmt.Printf("Error parsing game state: %v\n", err)
			return
		}
		
		p.gameState = gameState
		fmt.Println("Received updated game state")
		p.printGameState()
		
	case "private_cards":
		var cardsMsg struct {
			Type  string   `json:"type"`
			Cards []string `json:"cards"`
		}
		
		err := json.Unmarshal(msg, &cardsMsg)
		if err != nil {
			fmt.Printf("Error parsing private cards: %v\n", err)
			return
		}
		
		p.privateCards = cardsMsg.Cards
		fmt.Println("Received private cards: ", strings.Join(p.privateCards, ", "))
		
	case "public_cards":
		var cardsMsg struct {
			Type  string   `json:"type"`
			Cards []string `json:"cards"`
		}
		
		err := json.Unmarshal(msg, &cardsMsg)
		if err != nil {
			fmt.Printf("Error parsing public cards: %v\n", err)
			return
		}
		
		p.publicCards = cardsMsg.Cards
		fmt.Println("Public cards: ", strings.Join(p.publicCards, ", "))
		
	case "game_result":
		var resultMsg struct {
			Type    string `json:"type"`
			Winners []int  `json:"winners"`
		}
		
		err := json.Unmarshal(msg, &resultMsg)
		if err != nil {
			fmt.Printf("Error parsing game result: %v\n", err)
			return
		}
		
		fmt.Println("Game over. Winners: ", resultMsg.Winners)
		
	case "error":
		errorMsg, _ := msgMap["message"].(string)
		fmt.Printf("Error from server: %s\n", errorMsg)
		
	default:
		fmt.Printf("Unhandled message type: %s\n", msgType)
	}
}

// printGameState prints the current game state
func (p *PokerClient) printGameState() {
	fmt.Println("=== Game State ===")
	fmt.Printf("Stage: %d\n", p.gameState.Stage)
	fmt.Printf("Current Player: %d\n", p.gameState.CurrentPlayer)
	fmt.Printf("Dealer Position: %d\n", p.gameState.DealerPosition)
	
	if p.gameState.Pot > 0 {
		fmt.Printf("Pot: %d\n", p.gameState.Pot)
	}
	
	fmt.Println("Players:")
	for _, player := range p.gameState.Players {
		status := "Inactive"
		if player.IsActive {
			status = "Active"
		}
		if player.ID == p.gameState.CurrentPlayer {
			status += " (Current)"
		}
		
		ready := ""
		if player.IsReady {
			ready = " (Ready)"
		}
		
		fmt.Printf(" - ID: %d, Name: %s, Chips: %d, Status: %s%s\n", 
			player.ID, player.Name, player.Chips, status, ready)
	}
	
	if len(p.privateCards) > 0 {
		fmt.Printf("Your cards: %s\n", strings.Join(p.privateCards, ", "))
	}
	
	if len(p.publicCards) > 0 {
		fmt.Printf("Community cards: %s\n", strings.Join(p.publicCards, ", "))
	}
	
	fmt.Println("=================")
}

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Usage: go run poker_client.go <player_name> [server_ip] [port]")
		os.Exit(1)
	}
	
	playerName := os.Args[1]
	serverIP := "127.0.0.1" // Default to localhost
	port := 8080            // Default port
	
	if len(os.Args) > 2 {
		serverIP = os.Args[2]
	}
	
	if len(os.Args) > 3 {
		var err error
		port, err = strconv.Atoi(os.Args[3])
		if err != nil {
			fmt.Printf("Invalid port number: %s\n", os.Args[3])
			os.Exit(1)
		}
	}
	
	// Create client
	client := NewPokerClient(playerName)
	
	// Connect to server
	err := client.Connect(serverIP, port)
	if err != nil {
		fmt.Printf("Failed to connect: %v\n", err)
		os.Exit(1)
	}
	
	// Set up signal handling for clean shutdown
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)
	
	// Start goroutine to handle user input
	go func() {
		scanner := bufio.NewScanner(os.Stdin)
		
		fmt.Println("\nAvailable commands:")
		fmt.Println("ready - Set yourself as ready to play")
		fmt.Println("notready - Set yourself as not ready")
		fmt.Println("fold - Fold your hand")
		fmt.Println("check - Check")
		fmt.Println("call - Call the current bet")
		fmt.Println("raise <amount> - Raise by the specified amount")
		fmt.Println("quit - Disconnect and exit")
		
		for scanner.Scan() {
			input := scanner.Text()
			
			if input == "quit" {
				sigChan <- syscall.SIGTERM
				break
			} else if input == "ready" {
				err := client.SetReady(true)
				if err != nil {
					fmt.Printf("Error: %v\n", err)
				}
			} else if input == "notready" {
				err := client.SetReady(false)
				if err != nil {
					fmt.Printf("Error: %v\n", err)
				}
			} else if input == "fold" {
				err := client.PerformAction("fold", 0)
				if err != nil {
					fmt.Printf("Error: %v\n", err)
				}
			} else if input == "check" {
				err := client.PerformAction("check", 0)
				if err != nil {
					fmt.Printf("Error: %v\n", err)
				}
			} else if input == "call" {
				err := client.PerformAction("call", 0)
				if err != nil {
					fmt.Printf("Error: %v\n", err)
				}
			} else if strings.HasPrefix(input, "raise ") {
				parts := strings.Split(input, " ")
				if len(parts) != 2 {
					fmt.Println("Usage: raise <amount>")
					continue
				}
				
				amount, err := strconv.Atoi(parts[1])
				if err != nil {
					fmt.Printf("Invalid amount: %s\n", parts[1])
					continue
				}
				
				err = client.PerformAction("raise", amount)
				if err != nil {
					fmt.Printf("Error: %v\n", err)
				}
			} else {
				fmt.Println("Unknown command. Type 'quit' to exit.")
			}
		}
	}()
	
	// Wait for signal
	<-sigChan
	
	fmt.Println("Shutting down...")
	client.Disconnect()
} 