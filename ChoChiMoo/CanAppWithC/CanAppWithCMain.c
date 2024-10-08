#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <time.h>
#include <sys/select.h>

#define BUFFER_SIZE 256

// COBS 디코딩 함수 (단순 구현)
// 입력 데이터(input)를 COBS 방식으로 디코딩하여 출력(output)에 저장하는 함수
// 입력: input - COBS 인코딩된 데이터
//      length - 입력 데이터의 길이
// 출력: output - 디코딩된 데이터
// 반환 값: 디코딩된 데이터의 길이, 오류 시 -1 반환
int cobs_decode(const unsigned char *input, int length, unsigned char *output) {
    int read_index = 0, write_index = 0;
    unsigned char code;

    while (read_index < length) {
        code = input[read_index++];
        if (code == 0 || read_index + code > length + 1) {
            return -1; // 오류 처리: 잘못된 COBS 데이터
        }
        for (int i = 1; i < code; i++) {
            output[write_index++] = input[read_index++];
        }
        if (code < 0xFF && read_index < length) {
            output[write_index++] = 0; // 코드가 0xFF보다 작을 경우, 중간에 0 추가
        }
    }
    return write_index;
}

// 데이터 수신 이벤트 핸들러 함수
void handle_received_data(int usb_serial) {
    unsigned char read_buf[BUFFER_SIZE];
    unsigned char decoded_buf[BUFFER_SIZE];
    int num_bytes;

    memset(read_buf, 0, BUFFER_SIZE); // 읽기 버퍼 초기화
    num_bytes = read(usb_serial, read_buf, BUFFER_SIZE - 1); // 시리얼 포트에서 데이터 읽기

    if (num_bytes < 0) {
        perror("Error reading from serial port"); // 읽기 오류 발생 시
    } else if (num_bytes > 0) {
        read_buf[num_bytes] = '\0'; // 문자열 종료 문자 추가
        printf("Raw data received: %s\n", read_buf); // 수신된 원시 데이터 출력
        int decoded_length = cobs_decode(read_buf, num_bytes, decoded_buf); // COBS 디코딩 수행
        if (decoded_length > 0) {
            decoded_buf[decoded_length] = '\0'; // 디코딩된 데이터의 문자열 종료 문자 추가
            printf("Decoded packet: %s\n", decoded_buf); // 디코딩된 데이터 출력
        } else {
            printf("Error decoding COBS data\n"); // 디코딩 오류 발생 시
        }
    }
}

int main() {
    int usb_serial;
    struct termios tty;
    char serial_port[100];

    // 시리얼 포트 입력받기
    // 사용자가 프로그램 실행 시 시리얼 포트 경로를 입력하도록 요청
    printf("Enter the serial port (e.g., /dev/ttyACM0): ");
    scanf("%99s", serial_port);

    // USB 시리얼 포트 열기
    // 사용자가 입력한 시리얼 포트를 엽니다
    usb_serial = open(serial_port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (usb_serial == -1) {
        perror("Unable to open serial port");
        return 1;
    }

    // 시리얼 포트 설정
    // 시리얼 포트의 속성(struct termios)을 설정합니다
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(usb_serial, &tty) != 0) {
        perror("Error from tcgetattr");
        close(usb_serial);
        return 1;
    }

    // 시리얼 포트 속도 설정 (115200 baud rate)
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    // 시리얼 포트 설정 구성
    tty.c_cflag |= (CLOCAL | CREAD);    // 로컬 연결, 리시버 활성화
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;                 // 데이터 비트 8비트 설정
    tty.c_cflag &= ~PARENB;             // 패리티 비트 사용 안 함
    tty.c_cflag &= ~CSTOPB;             // 스톱 비트 1비트 설정
    tty.c_cflag &= ~CRTSCTS;            // 하드웨어 흐름 제어 사용 안 함

    // 로컬 플래그, 출력 플래그, 입력 플래그 초기화
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_iflag = 0;
    tty.c_cc[VMIN] = 1;                 // 최소 읽기 개수 설정
    tty.c_cc[VTIME] = 5;                // 읽기 타임아웃 (0.5초)

    // 시리얼 포트 속성 설정 적용
    if (tcsetattr(usb_serial, TCSANOW, &tty) != 0) {
        perror("Error from tcsetattr");
        close(usb_serial);
        return 1;
    }

    // 파일 디스크립터를 사용한 select()를 통한 이벤트 기반 데이터 처리
    fd_set read_fds;
    struct timeval timeout;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(usb_serial, &read_fds);

        timeout.tv_sec = 1;  // 타임아웃 1초 설정
        timeout.tv_usec = 0;

        int activity = select(usb_serial + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0) {
            perror("select error");
        } else if (activity > 0) {
            if (FD_ISSET(usb_serial, &read_fds)) {
                handle_received_data(usb_serial); // 데이터 수신 시 이벤트 핸들러 호출
            }
        } else {
            printf("No data received for last 1 second\n"); // 1초 동안 데이터가 없을 경우 에러 메시지 표시
        }
    }

    // 시리얼 포트 닫기
    close(usb_serial);
    return 0;
}