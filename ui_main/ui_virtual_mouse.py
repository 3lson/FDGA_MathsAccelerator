import cv2
import numpy as np
from cvzone.HandTrackingModule import HandDetector
import pyautogui
import time
from collections import deque

class VirtualMouse:
    def __init__(self):
        # Hand tracking
        self.detector = HandDetector(detectionCon=0.8, maxHands=2)
        
        # Gesture state tracking
        self.gesture_cooldown = 0
        self.cooldown_frames = 15
        self.prev_fingers = None
        
        # Index finger gesture tracking (for tap vs hold)
        self.index_down_start_time = 0
        self.index_was_down = False
        self.tap_threshold = 10  # frames - quick tap vs hold
        self.hold_threshold = 35  # frames - time needed for hold action
        self.index_hold_confirmed = False
        
        # Middle finger gesture tracking (for draw mode)
        self.middle_down_start_time = 0
        self.middle_was_down = False
        self.middle_hold_threshold = 15  # frames - faster activation for drawing
        self.middle_hold_confirmed = False
        self.draw_mode_active = False
        self.right_click_held = False
        
        # Mouse position smoothing
        self.mouse_history = deque(maxlen=5)
        self.mouse_smoothing = 0.3
        
        # Hand detection status
        self.hands_detected = False
        self.no_hands_message_time = 0
        
        # Exit confirmation
        self.exit_hold_time = 0
        self.exit_hold_required = 30
        
        # Screen dimensions
        self.screen_width, self.screen_height = pyautogui.size()
        
        # Camera dimensions (for mapping)
        self.cam_width = 640
        self.cam_height = 480
        
        # Safety settings
        pyautogui.FAILSAFE = True
        pyautogui.PAUSE = 0.01
        
        print("Virtual Mouse Controller with Draw Mode")
        print("=" * 50)
        print("Hand Gestures:")
        print("   • Peace sign (index + middle UP) = Move mouse cursor")
        print("   • From cursor mode: DROP index finger = LEFT CLICK")
        print("   • From cursor mode: DROP middle finger (HOLD) = RIGHT CLICK & DRAW")
        print("   • BOTH hands all fingers UP (hold 1s) = EXIT")
        print("")
        print("Workflow:")
        print("   1. Use peace sign to move mouse cursor")
        print("   2. Drop index finger briefly to left-click")
        print("   3. Drop middle finger and hold to enter draw mode (right-click)")
        print("   4. While in draw mode, move cursor to draw")
        print("   5. Return to peace sign to exit draw mode")
        print("")
        print("Press 'q' to force quit")

    def smooth_mouse_position(self, new_x, new_y):
        """Apply smoothing to mouse position to reduce jitter"""
        self.mouse_history.append((new_x, new_y))
        
        if len(self.mouse_history) < 2:
            return new_x, new_y
        
        weights = np.array([0.1, 0.15, 0.2, 0.25, 0.3])[-len(self.mouse_history):]
        weights = weights / weights.sum()
        
        avg_x = sum(pos[0] * weight for pos, weight in zip(self.mouse_history, weights))
        avg_y = sum(pos[1] * weight for pos, weight in zip(self.mouse_history, weights))
        
        return int(avg_x), int(avg_y)
    
    def move_mouse(self, finger_tip):
        """Move mouse cursor based on finger position"""
        if finger_tip is not None:
            # Map finger position to screen coordinates
            x = int(np.interp(finger_tip[0], [0, self.cam_width], [0, self.screen_width]))
            y = int(np.interp(finger_tip[1], [0, self.cam_height], [0, self.screen_height]))
            
            # Apply smoothing
            smooth_x, smooth_y = self.smooth_mouse_position(x, y)
            
            # Ensure coordinates are within screen bounds
            smooth_x = max(0, min(self.screen_width - 1, smooth_x))
            smooth_y = max(0, min(self.screen_height - 1, smooth_y))
            
            # Move the actual mouse cursor
            pyautogui.moveTo(smooth_x, smooth_y)
            
            return smooth_x, smooth_y
        return None, None
    
    def process_index_finger_gesture(self, current_fingers):
        """Process index finger tap vs hold gestures"""
        index_is_down = self.validate_finger_state(current_fingers, [0, 0, 1, 0, 0])
    
        if not hasattr(self, 'was_peace_sign'):
            self.was_peace_sign = False
    
        peace_sign = [0, 1, 1, 0, 0]
        if current_fingers == peace_sign:
            self.was_peace_sign = True
    
        if index_is_down and not self.was_peace_sign:
            return None
    
        if index_is_down:
            if not self.index_was_down:
                self.index_down_start_time = 0
                self.index_was_down = True
                self.index_hold_confirmed = False
            else:
                self.index_down_start_time += 1
            
            if (self.index_down_start_time >= self.hold_threshold and 
                not self.index_hold_confirmed):
                self.index_hold_confirmed = True
                return "HOLD"
        else:
            if self.index_was_down:
                if (self.index_down_start_time < self.tap_threshold and 
                    not self.index_hold_confirmed):
                    self.index_was_down = False
                    self.index_down_start_time = 0
                    self.was_peace_sign = False
                    return "TAP"
                else:
                    self.index_was_down = False
                    self.index_down_start_time = 0
                    self.index_hold_confirmed = False
                    self.was_peace_sign = False
        
        return None
    
    def process_middle_finger_gesture(self, current_fingers):
        """Process middle finger hold gestures for draw mode"""
        # Check if only middle finger is down from peace sign position
        middle_is_down = self.validate_finger_state(current_fingers, [0, 1, 0, 0, 0])
        
        if not hasattr(self, 'was_peace_sign_for_middle'):
            self.was_peace_sign_for_middle = False
        
        peace_sign = [0, 1, 1, 0, 0]
        if current_fingers == peace_sign:
            self.was_peace_sign_for_middle = True
        
        # Only allow middle finger gesture if we came from peace sign
        if middle_is_down and not self.was_peace_sign_for_middle:
            return None
        
        if middle_is_down:
            if not self.middle_was_down:
                self.middle_down_start_time = 0
                self.middle_was_down = True
                self.middle_hold_confirmed = False
            else:
                self.middle_down_start_time += 1
            
            # Check if hold threshold is reached
            if (self.middle_down_start_time >= self.middle_hold_threshold and 
                not self.middle_hold_confirmed):
                self.middle_hold_confirmed = True
                return "MIDDLE_HOLD_START"
            elif self.middle_hold_confirmed:
                return "MIDDLE_HOLDING"
        else:
            if self.middle_was_down and self.middle_hold_confirmed:
                # Middle finger released after hold
                self.middle_was_down = False
                self.middle_down_start_time = 0
                self.middle_hold_confirmed = False
                self.was_peace_sign_for_middle = False
                return "MIDDLE_HOLD_END"
            elif self.middle_was_down:
                # Middle finger released before hold threshold
                self.middle_was_down = False
                self.middle_down_start_time = 0
                self.middle_hold_confirmed = False
        
        return None
    
    def validate_finger_state(self, fingers, required_pattern, tolerance_frames=3):
        """Validate finger state with temporal consistency"""
        if not hasattr(self, 'finger_state_history'):
            self.finger_state_history = deque(maxlen=tolerance_frames)
    
        self.finger_state_history.append(fingers)
    
        if len(self.finger_state_history) < 2:
            return False
    
        pattern_matches = sum(1 for state in self.finger_state_history if state == required_pattern)
        return pattern_matches >= (len(self.finger_state_history) // 2)
    
    def handle_draw_mode(self, action):
        """Handle draw mode activation and deactivation"""
        if action == "MIDDLE_HOLD_START":
            if not self.draw_mode_active:
                # Start draw mode - perform right click and hold
                pyautogui.mouseDown(button='right')
                self.draw_mode_active = True
                self.right_click_held = True
                print("Draw mode activated - Right click held")
                return "DRAW MODE START"
        
        elif action == "MIDDLE_HOLDING":
            if self.draw_mode_active:
                return "DRAW MODE ACTIVE"
        
        elif action == "MIDDLE_HOLD_END":
            if self.draw_mode_active:
                # End draw mode - release right click
                if self.right_click_held:
                    pyautogui.mouseUp(button='right')
                    self.right_click_held = False
                self.draw_mode_active = False
                print("Draw mode deactivated - Right click released")
                return "DRAW MODE END"
        
        return None
    
    def draw_status_overlay(self, frame, current_gesture, cursor_active, exit_gesture_detected, mouse_pos):
        """Draw status information on camera frame"""
        # Draw semi-transparent overlay
        overlay = frame.copy()
        overlay_height = 140 if self.draw_mode_active else 120
        cv2.rectangle(overlay, (10, 10), (320, overlay_height), (0, 0, 0), -1)
        cv2.addWeighted(overlay, 0.7, frame, 0.3, 0, frame)
        
        # Status text
        status_texts = [
            f"Mouse Control: {'ACTIVE' if cursor_active else 'INACTIVE'}",
            f"Action: {current_gesture}",
            f"Mouse Position: {mouse_pos if mouse_pos[0] else 'N/A'}",
            f"Screen: {self.screen_width}x{self.screen_height}"
        ]
        
        # Add draw mode status
        if self.draw_mode_active:
            status_texts.append(f"DRAW MODE: ACTIVE (Right-click held)")
        
        for i, text in enumerate(status_texts):
            color = (0, 255, 0) if "ACTIVE" in text else (255, 255, 255)
            if "DRAW MODE" in text:
                color = (0, 255, 255)  # Yellow for draw mode
            cv2.putText(frame, text, (15, 30 + i * 20), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)
        
        # Draw middle finger hold progress bar if needed
        if self.middle_was_down and self.middle_down_start_time > 0 and not self.middle_hold_confirmed:
            bar_width = 200
            bar_height = 8
            bar_x = (frame.shape[1] - bar_width) // 2
            bar_y = 50
            
            cv2.rectangle(frame, (bar_x, bar_y), (bar_x + bar_width, bar_y + bar_height), (100, 100, 100), -1)
            
            progress = min(self.middle_down_start_time / self.middle_hold_threshold, 1.0)
            progress_width = int(progress * bar_width)
            
            color = (0, 255, 255) if progress < 1.0 else (0, 255, 0)
            cv2.rectangle(frame, (bar_x, bar_y), (bar_x + progress_width, bar_y + bar_height), color, -1)
            
            text = "Draw Mode Activating..." if progress < 1.0 else "Draw Mode Ready"
            cv2.putText(frame, text, (bar_x, bar_y - 5), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 255, 255), 1)
        
        # Draw exit progress bar if needed
        if exit_gesture_detected and self.exit_hold_time > 0:
            bar_width = 200
            bar_height = 12
            bar_x = (frame.shape[1] - bar_width) // 2
            bar_y = 80
            
            cv2.rectangle(frame, (bar_x, bar_y), (bar_x + bar_width, bar_y + bar_height), (100, 100, 100), -1)
            progress_width = int((self.exit_hold_time / self.exit_hold_required) * bar_width)
            cv2.rectangle(frame, (bar_x, bar_y), (bar_x + progress_width, bar_y + bar_height), (0, 0, 255), -1)
            cv2.putText(frame, "Hold both hands up to exit", (bar_x - 40, bar_y - 5), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 255, 255), 1)

    def draw_instructions(self, frame):
        """Draw instructions at bottom of frame"""
        instructions = [
            "Peace sign: Control mouse",
            "Drop INDEX finger: LEFT CLICK",
            "Drop MIDDLE finger (hold): DRAW MODE",
            "Both hands up: Exit"
        ]
        
        # Semi-transparent background
        overlay = frame.copy()
        start_y = frame.shape[0] - 100
        cv2.rectangle(overlay, (0, start_y), (frame.shape[1], frame.shape[0]), (0, 0, 0), -1)
        cv2.addWeighted(overlay, 0.7, frame, 0.3, 0, frame)
        
        for i, instruction in enumerate(instructions):
            y_pos = start_y + 20 + i * 18
            color = (0, 255, 255) if "DRAW MODE" in instruction and self.draw_mode_active else (255, 255, 255)
            cv2.putText(frame, instruction, (10, y_pos), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1)
    
    def cleanup_draw_mode(self):
        """Clean up draw mode if still active"""
        if self.right_click_held:
            try:
                pyautogui.mouseUp(button='right')
                print("Cleaned up: Released right click")
            except:
                pass
            self.right_click_held = False
        self.draw_mode_active = False
    
    def run(self):
        """Main application loop"""
        try:
            cap = cv2.VideoCapture(0)
            if not cap.isOpened():
                print("Error: Could not open camera")
                return
            
            cap.set(3, self.cam_width)
            cap.set(4, self.cam_height)
            
            while True:
                success, frame = cap.read()
                if not success:
                    print("Error: Failed to read from camera")
                    break
                
                frame = cv2.flip(frame, 1)
                
                try:
                    hands, frame = self.detector.findHands(frame, flipType=False)
                except Exception as e:
                    print(f"Hand detection error: {e}")
                    hands = []
                
                self.hands_detected = len(hands) > 0
                if not self.hands_detected:
                    self.no_hands_message_time = time.time()
                
                current_gesture = "None"
                exit_gesture_detected = False
                cursor_active = False
                mouse_pos = (None, None)
                
                if hands and self.gesture_cooldown == 0:
                    # Check for exit gesture first (both hands, all fingers up)
                    if len(hands) == 2:
                        try:
                            both_hands_all_fingers = all(
                                sum(self.detector.fingersUp(hand)) == 5 for hand in hands
                            )
                            if both_hands_all_fingers:
                                exit_gesture_detected = True
                                self.exit_hold_time += 1
                                current_gesture = f"EXITING... {self.exit_hold_time}/{self.exit_hold_required}"
                                
                                if self.exit_hold_time >= self.exit_hold_required:
                                    print("Exit confirmed")
                                    break
                        except Exception as e:
                            print(f"Error processing exit gesture: {e}")
                    
                    if not exit_gesture_detected:
                        self.exit_hold_time = 0
                    
                    # Process single hand gestures
                    if len(hands) >= 1 and not exit_gesture_detected:
                        try:
                            hand = hands[0]
                            fingers = self.detector.fingersUp(hand)
                            
                            # Process middle finger gesture for draw mode
                            middle_action = self.process_middle_finger_gesture(fingers)
                            draw_result = self.handle_draw_mode(middle_action)
                            
                            if draw_result:
                                current_gesture = draw_result
                                if "START" in draw_result or "END" in draw_result:
                                    self.gesture_cooldown = self.cooldown_frames
                            
                            # Process index finger tap/hold gestures (only if not in draw mode)
                            if not self.draw_mode_active:
                                index_action = self.process_index_finger_gesture(fingers)
                                
                                if index_action == "TAP":
                                    # Perform left click at current mouse position
                                    pyautogui.click()
                                    current_gesture = "LEFT CLICK!"
                                    print(f"Left click performed")
                                    self.gesture_cooldown = self.cooldown_frames
                                                                
                                elif index_action == "HOLD":
                                    current_gesture = "HOLD DETECTED"
                                    self.gesture_cooldown = self.cooldown_frames
                            
                            # Handle cursor mode (peace sign or draw mode)
                            if fingers == [0, 1, 1, 0, 0] or self.draw_mode_active:
                                cursor_active = True
                                if self.draw_mode_active:
                                    current_gesture = "DRAW MODE ACTIVE"
                                elif current_gesture == "None" or "MOUSE" in current_gesture:
                                    current_gesture = "MOUSE CONTROL"
                                
                                # Use middle finger tip for mouse control in peace sign
                                # Use index finger tip for mouse control in draw mode
                                if self.draw_mode_active:
                                    finger_tip = hand["lmList"][8]  # Index finger tip for draw mode
                                else:
                                    finger_tip = hand["lmList"][12]  # Middle finger tip for normal mode
                                mouse_pos = self.move_mouse(finger_tip)
                        
                        except Exception as e:
                            print(f"Error processing hand gesture: {e}")
                            current_gesture = "GESTURE ERROR"
                
                # Reset states if no hands detected
                if not hands:
                    self.exit_hold_time = 0
                    self.prev_fingers = None
                    self.index_was_down = False
                    self.index_down_start_time = 0
                    self.index_hold_confirmed = False
                    
                    # Clean up draw mode if no hands detected
                    if self.draw_mode_active:
                        self.cleanup_draw_mode()
                        current_gesture = "DRAW MODE ENDED (No hands)"
                
                # Decrease cooldown
                if self.gesture_cooldown > 0:
                    self.gesture_cooldown -= 1
                
                # Draw status overlay
                self.draw_status_overlay(frame, current_gesture, cursor_active, exit_gesture_detected, mouse_pos)
                
                # Draw instructions
                self.draw_instructions(frame)
                
                # Show no hands warning
                if not self.hands_detected and time.time() - self.no_hands_message_time < 3:
                    warning_y = frame.shape[0] // 2
                    cv2.putText(frame, "NO HANDS DETECTED", (frame.shape[1] // 2 - 100, warning_y), 
                               cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 255), 2)
                
                # Display frame
                cv2.imshow("Virtual Mouse Controller", frame)
                
                if cv2.waitKey(1) & 0xFF == ord('q'):
                    break
        
        except Exception as e:
            print(f"Critical error: {e}")
        finally:
            # Ensure draw mode is cleaned up
            self.cleanup_draw_mode()
            
            try:
                cap.release()
                cv2.destroyAllWindows()
            except:
                pass
            
            print("Virtual mouse controller stopped")

if __name__ == "__main__":
    try:
        controller = VirtualMouse()
        controller.run()
    except KeyboardInterrupt:
        print("\nApplication interrupted by user")
        # Ensure cleanup on keyboard interrupt
        try:
            pyautogui.mouseUp(button='right')
        except:
            pass
    except Exception as e:
        print(f"Application failed to start: {e}")
        print("Please check required packages: pip install opencv-python cvzone pyautogui")