#include "Request.hpp"

void printReqInfo(std::string str, Request& req)
{
    std::string tmp(str);

    std::cout<<tmp<<std::endl;
    if (req.getReqInfo() == ReqInfo::READY)
        std::cout<<"ReqInfo: READY"<<std::endl;
    else if (req.getReqInfo() == ReqInfo::COMPLETE)
        std::cout<<"ReqInfo: COMPLETE"<<std::endl;
    else if (req.getReqInfo() == ReqInfo::NORMAL_BODY)
        std::cout<<"ReqInfo: NORMAL_BODY"<<std::endl;
    else if (req.getReqInfo() == ReqInfo::CHUNKED_BODY)
        std::cout<<"ReqInfo: CHUNKED_BODY"<<std::endl;
}

int main()
{
    Request req_ready;
    Request req_completed;
    Request req_normal;
    Request req_chunked;

    req_completed.setReqInfo(ReqInfo::COMPLETE);
    req_normal.setReqInfo(ReqInfo::NORMAL_BODY);
    req_chunked.setReqInfo(ReqInfo::CHUNKED_BODY);

    req_ready.updateReqInfo();
    std::cout<<"Not yet requested"<<std::endl;
    printReqInfo("", req_ready);
    req_ready.setMethod(std::string("PUT"));
    req_ready.setHeaders("Transfer-Encoding", "chunked");
    std::cout<<"Now method is "<<req_ready.getMethod()<<std::endl;
    for (auto& kv : req_ready.getHeaders())
	{
		std::cout << "Key: " << kv.first << " value: " << kv.second << std::endl;
	}
    printReqInfo("before update", req_ready); //READY
    req_ready.updateReqInfo();
    printReqInfo("after update", req_ready); //NORMAL_BODY

    return (0);
}