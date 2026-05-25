## Core Module: Emotion & Expression Engine (감정 연산 및 상태 제어)

본 모듈은 여러 보드 및 센서에서 수집된 비동기 데이터를 종합하여 펫 로봇의 감정(Emotion) 수치를 0~100으로 연산하고, 이를 실시간 표정으로 시각화하는 메인 제어 코어입니다.

- **MCU / Tech Stack:** STM32F411, C, I2C(OLED), UART(Bluetooth & Inter-MCU), GPIO
- **Role:** 다중 센서 융합 기반 감정 추론 알고리즘 및 Dual OLED 표정 동기화 구현

### 주요 구현 로직 (Key Features)

**1. 다중 데이터 융합 및 감정 연산 (Emotion Calculation)**
- **사용자 상호작용 반영:** Interaction Board로부터 UART6 통신을 통해 터치, 먹이 주기, 놀아주기 등의 이벤트를 수신하여 감정 수치를 증감시킵니다.

- **환경 모니터링 (DHT11):** 온도 30°C 이상 또는 습도 70% 이상 감지 시 '환경 악화'로 판단하여 감정 수치를 즉각 및 주기적으로(20초) 감소시키는 환경 인지 로직을 구현했습니다.

- **Starvation Mode 제어:** 허기짐이나 지루함 상태(Hold=1) 수신 시, 20초 주기로 감정이 자동 감소하는 Auto-decrease 루프를 설계하여 살아있는 펫의 생체 주기를 모사했습니다.

	- **State 2 [Feed]:** RFID 기반 포만감 회복 (먹이 주기)
	- **State 3 [Play]:** RFID 기반 기쁨 수치 회복 (놀아주기)
	- **State 5 [Touch]:** 압력 센서 기반 물리적 상호작용 (쓰다듬기)
	- **Hold State [Starvation]:** 생체 주기 모사를 위한 감정 결핍 상태 (자동 Emotion 값 감소 루프 진입 트리거)

**2. 조건부 상태 전이 제어 (Conditional State Transition)**
- 로봇의 현재 감정 상태(Emotion Level)에 따라 사용자의 상호작용에 대한 가중치를 차등 적용했습니다. 

- *예외 처리:* 감정 수치가 25 미만(Bad)일 경우, 10번을 터치하더라도 감정 수치가 오르지 않도록 설정하여 단순 반복 동작을 방지하고 상호작용의 현실감을 부여했습니다.

**3. 실시간 상태 시각화 및 로깅 (Real-time Expression & Logging)**
- **상태 분류:** 연산된 감정 수치(0~100)를 기준으로 3단계 상태(Bad: <25, Normal: 25-69, Happy: 70+)로 분류하는 FSM 기반 상태 머신을 구축했습니다.

- **Dual OLED 제어:** I2C 통신을 통해 2개의 SSD1306 OLED 디스플레이를 제어하며, 현재 감정 상태에 맞는 표정 픽셀(Font5x7)을 즉각적으로 렌더링합니다.

- **상태 동기화:** 감정 상태가 변할 때마다 Bluetooth(UART1)를 통해 상태 로그 메시지를 송출하여 디버깅 및 모니터링 환경을 확보했습니다.
