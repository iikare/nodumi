#!/bin/zsh

C_F="\033[31;1m"
C_S="\033[34;1m"
C_I="\033[32m"
C_E="\033[0m"

cecho() {
  echo "$1$2$C_E"
}
