#pragma once

#include <string>

int AddChatroomMember(std::string roomid, std::string wxids);
int DelChatroomMember(std::string roomid, std::string wxids);
int InviteChatroomMember(std::string roomid, std::string wxids);
