// GENERATED CODE -- DO NOT EDIT!

'use strict';
var grpc = require('grpc');
var wcf_pb = require('./wcf_pb.js');

function serialize_wcf_Contacts(arg) {
  if (!(arg instanceof wcf_pb.Contacts)) {
    throw new Error('Expected argument of type wcf.Contacts');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_Contacts(buffer_arg) {
  return wcf_pb.Contacts.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_DbNames(arg) {
  if (!(arg instanceof wcf_pb.DbNames)) {
    throw new Error('Expected argument of type wcf.DbNames');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_DbNames(buffer_arg) {
  return wcf_pb.DbNames.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_DbQuery(arg) {
  if (!(arg instanceof wcf_pb.DbQuery)) {
    throw new Error('Expected argument of type wcf.DbQuery');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_DbQuery(buffer_arg) {
  return wcf_pb.DbQuery.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_DbRows(arg) {
  if (!(arg instanceof wcf_pb.DbRows)) {
    throw new Error('Expected argument of type wcf.DbRows');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_DbRows(buffer_arg) {
  return wcf_pb.DbRows.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_DbTables(arg) {
  if (!(arg instanceof wcf_pb.DbTables)) {
    throw new Error('Expected argument of type wcf.DbTables');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_DbTables(buffer_arg) {
  return wcf_pb.DbTables.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_Empty(arg) {
  if (!(arg instanceof wcf_pb.Empty)) {
    throw new Error('Expected argument of type wcf.Empty');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_Empty(buffer_arg) {
  return wcf_pb.Empty.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_ImageMsg(arg) {
  if (!(arg instanceof wcf_pb.ImageMsg)) {
    throw new Error('Expected argument of type wcf.ImageMsg');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_ImageMsg(buffer_arg) {
  return wcf_pb.ImageMsg.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_MsgTypes(arg) {
  if (!(arg instanceof wcf_pb.MsgTypes)) {
    throw new Error('Expected argument of type wcf.MsgTypes');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_MsgTypes(buffer_arg) {
  return wcf_pb.MsgTypes.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_Response(arg) {
  if (!(arg instanceof wcf_pb.Response)) {
    throw new Error('Expected argument of type wcf.Response');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_Response(buffer_arg) {
  return wcf_pb.Response.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_String(arg) {
  if (!(arg instanceof wcf_pb.String)) {
    throw new Error('Expected argument of type wcf.String');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_String(buffer_arg) {
  return wcf_pb.String.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_TextMsg(arg) {
  if (!(arg instanceof wcf_pb.TextMsg)) {
    throw new Error('Expected argument of type wcf.TextMsg');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_TextMsg(buffer_arg) {
  return wcf_pb.TextMsg.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_Verification(arg) {
  if (!(arg instanceof wcf_pb.Verification)) {
    throw new Error('Expected argument of type wcf.Verification');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_Verification(buffer_arg) {
  return wcf_pb.Verification.deserializeBinary(new Uint8Array(buffer_arg));
}

function serialize_wcf_WxMsg(arg) {
  if (!(arg instanceof wcf_pb.WxMsg)) {
    throw new Error('Expected argument of type wcf.WxMsg');
  }
  return Buffer.from(arg.serializeBinary());
}

function deserialize_wcf_WxMsg(buffer_arg) {
  return wcf_pb.WxMsg.deserializeBinary(new Uint8Array(buffer_arg));
}


var WcfService = exports.WcfService = {
  rpcIsLogin: {
    path: '/wcf.Wcf/RpcIsLogin',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.Empty,
    responseType: wcf_pb.Response,
    requestSerialize: serialize_wcf_Empty,
    requestDeserialize: deserialize_wcf_Empty,
    responseSerialize: serialize_wcf_Response,
    responseDeserialize: deserialize_wcf_Response,
  },
  rpcGetSelfWxid: {
    path: '/wcf.Wcf/RpcGetSelfWxid',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.Empty,
    responseType: wcf_pb.String,
    requestSerialize: serialize_wcf_Empty,
    requestDeserialize: deserialize_wcf_Empty,
    responseSerialize: serialize_wcf_String,
    responseDeserialize: deserialize_wcf_String,
  },
  rpcEnableRecvMsg: {
    path: '/wcf.Wcf/RpcEnableRecvMsg',
    requestStream: false,
    responseStream: true,
    requestType: wcf_pb.Empty,
    responseType: wcf_pb.WxMsg,
    requestSerialize: serialize_wcf_Empty,
    requestDeserialize: deserialize_wcf_Empty,
    responseSerialize: serialize_wcf_WxMsg,
    responseDeserialize: deserialize_wcf_WxMsg,
  },
  rpcDisableRecvMsg: {
    path: '/wcf.Wcf/RpcDisableRecvMsg',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.Empty,
    responseType: wcf_pb.Response,
    requestSerialize: serialize_wcf_Empty,
    requestDeserialize: deserialize_wcf_Empty,
    responseSerialize: serialize_wcf_Response,
    responseDeserialize: deserialize_wcf_Response,
  },
  rpcSendTextMsg: {
    path: '/wcf.Wcf/RpcSendTextMsg',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.TextMsg,
    responseType: wcf_pb.Response,
    requestSerialize: serialize_wcf_TextMsg,
    requestDeserialize: deserialize_wcf_TextMsg,
    responseSerialize: serialize_wcf_Response,
    responseDeserialize: deserialize_wcf_Response,
  },
  rpcSendImageMsg: {
    path: '/wcf.Wcf/RpcSendImageMsg',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.ImageMsg,
    responseType: wcf_pb.Response,
    requestSerialize: serialize_wcf_ImageMsg,
    requestDeserialize: deserialize_wcf_ImageMsg,
    responseSerialize: serialize_wcf_Response,
    responseDeserialize: deserialize_wcf_Response,
  },
  rpcGetMsgTypes: {
    path: '/wcf.Wcf/RpcGetMsgTypes',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.Empty,
    responseType: wcf_pb.MsgTypes,
    requestSerialize: serialize_wcf_Empty,
    requestDeserialize: deserialize_wcf_Empty,
    responseSerialize: serialize_wcf_MsgTypes,
    responseDeserialize: deserialize_wcf_MsgTypes,
  },
  rpcGetContacts: {
    path: '/wcf.Wcf/RpcGetContacts',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.Empty,
    responseType: wcf_pb.Contacts,
    requestSerialize: serialize_wcf_Empty,
    requestDeserialize: deserialize_wcf_Empty,
    responseSerialize: serialize_wcf_Contacts,
    responseDeserialize: deserialize_wcf_Contacts,
  },
  rpcGetDbNames: {
    path: '/wcf.Wcf/RpcGetDbNames',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.Empty,
    responseType: wcf_pb.DbNames,
    requestSerialize: serialize_wcf_Empty,
    requestDeserialize: deserialize_wcf_Empty,
    responseSerialize: serialize_wcf_DbNames,
    responseDeserialize: deserialize_wcf_DbNames,
  },
  rpcGetDbTables: {
    path: '/wcf.Wcf/RpcGetDbTables',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.String,
    responseType: wcf_pb.DbTables,
    requestSerialize: serialize_wcf_String,
    requestDeserialize: deserialize_wcf_String,
    responseSerialize: serialize_wcf_DbTables,
    responseDeserialize: deserialize_wcf_DbTables,
  },
  rpcExecDbQuery: {
    path: '/wcf.Wcf/RpcExecDbQuery',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.DbQuery,
    responseType: wcf_pb.DbRows,
    requestSerialize: serialize_wcf_DbQuery,
    requestDeserialize: deserialize_wcf_DbQuery,
    responseSerialize: serialize_wcf_DbRows,
    responseDeserialize: deserialize_wcf_DbRows,
  },
  rpcAcceptNewFriend: {
    path: '/wcf.Wcf/RpcAcceptNewFriend',
    requestStream: false,
    responseStream: false,
    requestType: wcf_pb.Verification,
    responseType: wcf_pb.Response,
    requestSerialize: serialize_wcf_Verification,
    requestDeserialize: deserialize_wcf_Verification,
    responseSerialize: serialize_wcf_Response,
    responseDeserialize: deserialize_wcf_Response,
  },
};

exports.WcfClient = grpc.makeGenericClientConstructor(WcfService);
