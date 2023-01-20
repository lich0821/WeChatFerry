// package: wcf
// file: wcf.proto

/* tslint:disable */
/* eslint-disable */

import * as grpc from "grpc";
import * as wcf_pb from "./wcf_pb";

interface IWcfService extends grpc.ServiceDefinition<grpc.UntypedServiceImplementation> {
    rpcIsLogin: IWcfService_IRpcIsLogin;
    rpcGetSelfWxid: IWcfService_IRpcGetSelfWxid;
    rpcEnableRecvMsg: IWcfService_IRpcEnableRecvMsg;
    rpcDisableRecvMsg: IWcfService_IRpcDisableRecvMsg;
    rpcSendTextMsg: IWcfService_IRpcSendTextMsg;
    rpcSendImageMsg: IWcfService_IRpcSendImageMsg;
    rpcGetMsgTypes: IWcfService_IRpcGetMsgTypes;
    rpcGetContacts: IWcfService_IRpcGetContacts;
    rpcGetDbNames: IWcfService_IRpcGetDbNames;
    rpcGetDbTables: IWcfService_IRpcGetDbTables;
    rpcExecDbQuery: IWcfService_IRpcExecDbQuery;
    rpcAcceptNewFriend: IWcfService_IRpcAcceptNewFriend;
}

interface IWcfService_IRpcIsLogin extends grpc.MethodDefinition<wcf_pb.Empty, wcf_pb.Response> {
    path: "/wcf.Wcf/RpcIsLogin";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.Empty>;
    requestDeserialize: grpc.deserialize<wcf_pb.Empty>;
    responseSerialize: grpc.serialize<wcf_pb.Response>;
    responseDeserialize: grpc.deserialize<wcf_pb.Response>;
}
interface IWcfService_IRpcGetSelfWxid extends grpc.MethodDefinition<wcf_pb.Empty, wcf_pb.String> {
    path: "/wcf.Wcf/RpcGetSelfWxid";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.Empty>;
    requestDeserialize: grpc.deserialize<wcf_pb.Empty>;
    responseSerialize: grpc.serialize<wcf_pb.String>;
    responseDeserialize: grpc.deserialize<wcf_pb.String>;
}
interface IWcfService_IRpcEnableRecvMsg extends grpc.MethodDefinition<wcf_pb.Empty, wcf_pb.WxMsg> {
    path: "/wcf.Wcf/RpcEnableRecvMsg";
    requestStream: false;
    responseStream: true;
    requestSerialize: grpc.serialize<wcf_pb.Empty>;
    requestDeserialize: grpc.deserialize<wcf_pb.Empty>;
    responseSerialize: grpc.serialize<wcf_pb.WxMsg>;
    responseDeserialize: grpc.deserialize<wcf_pb.WxMsg>;
}
interface IWcfService_IRpcDisableRecvMsg extends grpc.MethodDefinition<wcf_pb.Empty, wcf_pb.Response> {
    path: "/wcf.Wcf/RpcDisableRecvMsg";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.Empty>;
    requestDeserialize: grpc.deserialize<wcf_pb.Empty>;
    responseSerialize: grpc.serialize<wcf_pb.Response>;
    responseDeserialize: grpc.deserialize<wcf_pb.Response>;
}
interface IWcfService_IRpcSendTextMsg extends grpc.MethodDefinition<wcf_pb.TextMsg, wcf_pb.Response> {
    path: "/wcf.Wcf/RpcSendTextMsg";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.TextMsg>;
    requestDeserialize: grpc.deserialize<wcf_pb.TextMsg>;
    responseSerialize: grpc.serialize<wcf_pb.Response>;
    responseDeserialize: grpc.deserialize<wcf_pb.Response>;
}
interface IWcfService_IRpcSendImageMsg extends grpc.MethodDefinition<wcf_pb.ImageMsg, wcf_pb.Response> {
    path: "/wcf.Wcf/RpcSendImageMsg";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.ImageMsg>;
    requestDeserialize: grpc.deserialize<wcf_pb.ImageMsg>;
    responseSerialize: grpc.serialize<wcf_pb.Response>;
    responseDeserialize: grpc.deserialize<wcf_pb.Response>;
}
interface IWcfService_IRpcGetMsgTypes extends grpc.MethodDefinition<wcf_pb.Empty, wcf_pb.MsgTypes> {
    path: "/wcf.Wcf/RpcGetMsgTypes";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.Empty>;
    requestDeserialize: grpc.deserialize<wcf_pb.Empty>;
    responseSerialize: grpc.serialize<wcf_pb.MsgTypes>;
    responseDeserialize: grpc.deserialize<wcf_pb.MsgTypes>;
}
interface IWcfService_IRpcGetContacts extends grpc.MethodDefinition<wcf_pb.Empty, wcf_pb.Contacts> {
    path: "/wcf.Wcf/RpcGetContacts";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.Empty>;
    requestDeserialize: grpc.deserialize<wcf_pb.Empty>;
    responseSerialize: grpc.serialize<wcf_pb.Contacts>;
    responseDeserialize: grpc.deserialize<wcf_pb.Contacts>;
}
interface IWcfService_IRpcGetDbNames extends grpc.MethodDefinition<wcf_pb.Empty, wcf_pb.DbNames> {
    path: "/wcf.Wcf/RpcGetDbNames";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.Empty>;
    requestDeserialize: grpc.deserialize<wcf_pb.Empty>;
    responseSerialize: grpc.serialize<wcf_pb.DbNames>;
    responseDeserialize: grpc.deserialize<wcf_pb.DbNames>;
}
interface IWcfService_IRpcGetDbTables extends grpc.MethodDefinition<wcf_pb.String, wcf_pb.DbTables> {
    path: "/wcf.Wcf/RpcGetDbTables";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.String>;
    requestDeserialize: grpc.deserialize<wcf_pb.String>;
    responseSerialize: grpc.serialize<wcf_pb.DbTables>;
    responseDeserialize: grpc.deserialize<wcf_pb.DbTables>;
}
interface IWcfService_IRpcExecDbQuery extends grpc.MethodDefinition<wcf_pb.DbQuery, wcf_pb.DbRows> {
    path: "/wcf.Wcf/RpcExecDbQuery";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.DbQuery>;
    requestDeserialize: grpc.deserialize<wcf_pb.DbQuery>;
    responseSerialize: grpc.serialize<wcf_pb.DbRows>;
    responseDeserialize: grpc.deserialize<wcf_pb.DbRows>;
}
interface IWcfService_IRpcAcceptNewFriend extends grpc.MethodDefinition<wcf_pb.Verification, wcf_pb.Response> {
    path: "/wcf.Wcf/RpcAcceptNewFriend";
    requestStream: false;
    responseStream: false;
    requestSerialize: grpc.serialize<wcf_pb.Verification>;
    requestDeserialize: grpc.deserialize<wcf_pb.Verification>;
    responseSerialize: grpc.serialize<wcf_pb.Response>;
    responseDeserialize: grpc.deserialize<wcf_pb.Response>;
}

export const WcfService: IWcfService;

export interface IWcfServer {
    rpcIsLogin: grpc.handleUnaryCall<wcf_pb.Empty, wcf_pb.Response>;
    rpcGetSelfWxid: grpc.handleUnaryCall<wcf_pb.Empty, wcf_pb.String>;
    rpcEnableRecvMsg: grpc.handleServerStreamingCall<wcf_pb.Empty, wcf_pb.WxMsg>;
    rpcDisableRecvMsg: grpc.handleUnaryCall<wcf_pb.Empty, wcf_pb.Response>;
    rpcSendTextMsg: grpc.handleUnaryCall<wcf_pb.TextMsg, wcf_pb.Response>;
    rpcSendImageMsg: grpc.handleUnaryCall<wcf_pb.ImageMsg, wcf_pb.Response>;
    rpcGetMsgTypes: grpc.handleUnaryCall<wcf_pb.Empty, wcf_pb.MsgTypes>;
    rpcGetContacts: grpc.handleUnaryCall<wcf_pb.Empty, wcf_pb.Contacts>;
    rpcGetDbNames: grpc.handleUnaryCall<wcf_pb.Empty, wcf_pb.DbNames>;
    rpcGetDbTables: grpc.handleUnaryCall<wcf_pb.String, wcf_pb.DbTables>;
    rpcExecDbQuery: grpc.handleUnaryCall<wcf_pb.DbQuery, wcf_pb.DbRows>;
    rpcAcceptNewFriend: grpc.handleUnaryCall<wcf_pb.Verification, wcf_pb.Response>;
}

export interface IWcfClient {
    rpcIsLogin(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcIsLogin(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcIsLogin(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcGetSelfWxid(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.String) => void): grpc.ClientUnaryCall;
    rpcGetSelfWxid(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.String) => void): grpc.ClientUnaryCall;
    rpcGetSelfWxid(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.String) => void): grpc.ClientUnaryCall;
    rpcEnableRecvMsg(request: wcf_pb.Empty, options?: Partial<grpc.CallOptions>): grpc.ClientReadableStream<wcf_pb.WxMsg>;
    rpcEnableRecvMsg(request: wcf_pb.Empty, metadata?: grpc.Metadata, options?: Partial<grpc.CallOptions>): grpc.ClientReadableStream<wcf_pb.WxMsg>;
    rpcDisableRecvMsg(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcDisableRecvMsg(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcDisableRecvMsg(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcSendTextMsg(request: wcf_pb.TextMsg, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcSendTextMsg(request: wcf_pb.TextMsg, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcSendTextMsg(request: wcf_pb.TextMsg, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcSendImageMsg(request: wcf_pb.ImageMsg, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcSendImageMsg(request: wcf_pb.ImageMsg, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcSendImageMsg(request: wcf_pb.ImageMsg, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcGetMsgTypes(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.MsgTypes) => void): grpc.ClientUnaryCall;
    rpcGetMsgTypes(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.MsgTypes) => void): grpc.ClientUnaryCall;
    rpcGetMsgTypes(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.MsgTypes) => void): grpc.ClientUnaryCall;
    rpcGetContacts(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.Contacts) => void): grpc.ClientUnaryCall;
    rpcGetContacts(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Contacts) => void): grpc.ClientUnaryCall;
    rpcGetContacts(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Contacts) => void): grpc.ClientUnaryCall;
    rpcGetDbNames(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbNames) => void): grpc.ClientUnaryCall;
    rpcGetDbNames(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbNames) => void): grpc.ClientUnaryCall;
    rpcGetDbNames(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbNames) => void): grpc.ClientUnaryCall;
    rpcGetDbTables(request: wcf_pb.String, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbTables) => void): grpc.ClientUnaryCall;
    rpcGetDbTables(request: wcf_pb.String, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbTables) => void): grpc.ClientUnaryCall;
    rpcGetDbTables(request: wcf_pb.String, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbTables) => void): grpc.ClientUnaryCall;
    rpcExecDbQuery(request: wcf_pb.DbQuery, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbRows) => void): grpc.ClientUnaryCall;
    rpcExecDbQuery(request: wcf_pb.DbQuery, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbRows) => void): grpc.ClientUnaryCall;
    rpcExecDbQuery(request: wcf_pb.DbQuery, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbRows) => void): grpc.ClientUnaryCall;
    rpcAcceptNewFriend(request: wcf_pb.Verification, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcAcceptNewFriend(request: wcf_pb.Verification, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    rpcAcceptNewFriend(request: wcf_pb.Verification, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
}

export class WcfClient extends grpc.Client implements IWcfClient {
    constructor(address: string, credentials: grpc.ChannelCredentials, options?: object);
    public rpcIsLogin(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcIsLogin(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcIsLogin(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcGetSelfWxid(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.String) => void): grpc.ClientUnaryCall;
    public rpcGetSelfWxid(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.String) => void): grpc.ClientUnaryCall;
    public rpcGetSelfWxid(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.String) => void): grpc.ClientUnaryCall;
    public rpcEnableRecvMsg(request: wcf_pb.Empty, options?: Partial<grpc.CallOptions>): grpc.ClientReadableStream<wcf_pb.WxMsg>;
    public rpcEnableRecvMsg(request: wcf_pb.Empty, metadata?: grpc.Metadata, options?: Partial<grpc.CallOptions>): grpc.ClientReadableStream<wcf_pb.WxMsg>;
    public rpcDisableRecvMsg(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcDisableRecvMsg(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcDisableRecvMsg(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcSendTextMsg(request: wcf_pb.TextMsg, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcSendTextMsg(request: wcf_pb.TextMsg, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcSendTextMsg(request: wcf_pb.TextMsg, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcSendImageMsg(request: wcf_pb.ImageMsg, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcSendImageMsg(request: wcf_pb.ImageMsg, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcSendImageMsg(request: wcf_pb.ImageMsg, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcGetMsgTypes(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.MsgTypes) => void): grpc.ClientUnaryCall;
    public rpcGetMsgTypes(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.MsgTypes) => void): grpc.ClientUnaryCall;
    public rpcGetMsgTypes(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.MsgTypes) => void): grpc.ClientUnaryCall;
    public rpcGetContacts(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.Contacts) => void): grpc.ClientUnaryCall;
    public rpcGetContacts(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Contacts) => void): grpc.ClientUnaryCall;
    public rpcGetContacts(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Contacts) => void): grpc.ClientUnaryCall;
    public rpcGetDbNames(request: wcf_pb.Empty, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbNames) => void): grpc.ClientUnaryCall;
    public rpcGetDbNames(request: wcf_pb.Empty, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbNames) => void): grpc.ClientUnaryCall;
    public rpcGetDbNames(request: wcf_pb.Empty, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbNames) => void): grpc.ClientUnaryCall;
    public rpcGetDbTables(request: wcf_pb.String, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbTables) => void): grpc.ClientUnaryCall;
    public rpcGetDbTables(request: wcf_pb.String, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbTables) => void): grpc.ClientUnaryCall;
    public rpcGetDbTables(request: wcf_pb.String, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbTables) => void): grpc.ClientUnaryCall;
    public rpcExecDbQuery(request: wcf_pb.DbQuery, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbRows) => void): grpc.ClientUnaryCall;
    public rpcExecDbQuery(request: wcf_pb.DbQuery, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbRows) => void): grpc.ClientUnaryCall;
    public rpcExecDbQuery(request: wcf_pb.DbQuery, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.DbRows) => void): grpc.ClientUnaryCall;
    public rpcAcceptNewFriend(request: wcf_pb.Verification, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcAcceptNewFriend(request: wcf_pb.Verification, metadata: grpc.Metadata, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
    public rpcAcceptNewFriend(request: wcf_pb.Verification, metadata: grpc.Metadata, options: Partial<grpc.CallOptions>, callback: (error: grpc.ServiceError | null, response: wcf_pb.Response) => void): grpc.ClientUnaryCall;
}
