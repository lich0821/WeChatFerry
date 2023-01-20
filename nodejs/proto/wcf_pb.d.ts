// package: wcf
// file: wcf.proto

/* tslint:disable */
/* eslint-disable */

import * as jspb from "google-protobuf";

export class Empty extends jspb.Message { 

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): Empty.AsObject;
    static toObject(includeInstance: boolean, msg: Empty): Empty.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: Empty, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): Empty;
    static deserializeBinaryFromReader(message: Empty, reader: jspb.BinaryReader): Empty;
}

export namespace Empty {
    export type AsObject = {
    }
}

export class WxMsg extends jspb.Message { 
    getIsSelf(): boolean;
    setIsSelf(value: boolean): WxMsg;
    getIsGroup(): boolean;
    setIsGroup(value: boolean): WxMsg;
    getType(): number;
    setType(value: number): WxMsg;
    getId(): string;
    setId(value: string): WxMsg;
    getXml(): string;
    setXml(value: string): WxMsg;
    getSender(): string;
    setSender(value: string): WxMsg;
    getRoomid(): string;
    setRoomid(value: string): WxMsg;
    getContent(): string;
    setContent(value: string): WxMsg;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): WxMsg.AsObject;
    static toObject(includeInstance: boolean, msg: WxMsg): WxMsg.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: WxMsg, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): WxMsg;
    static deserializeBinaryFromReader(message: WxMsg, reader: jspb.BinaryReader): WxMsg;
}

export namespace WxMsg {
    export type AsObject = {
        isSelf: boolean,
        isGroup: boolean,
        type: number,
        id: string,
        xml: string,
        sender: string,
        roomid: string,
        content: string,
    }
}

export class Response extends jspb.Message { 
    getStatus(): number;
    setStatus(value: number): Response;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): Response.AsObject;
    static toObject(includeInstance: boolean, msg: Response): Response.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: Response, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): Response;
    static deserializeBinaryFromReader(message: Response, reader: jspb.BinaryReader): Response;
}

export namespace Response {
    export type AsObject = {
        status: number,
    }
}

export class TextMsg extends jspb.Message { 
    getMsg(): string;
    setMsg(value: string): TextMsg;
    getReceiver(): string;
    setReceiver(value: string): TextMsg;
    getAters(): string;
    setAters(value: string): TextMsg;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): TextMsg.AsObject;
    static toObject(includeInstance: boolean, msg: TextMsg): TextMsg.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: TextMsg, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): TextMsg;
    static deserializeBinaryFromReader(message: TextMsg, reader: jspb.BinaryReader): TextMsg;
}

export namespace TextMsg {
    export type AsObject = {
        msg: string,
        receiver: string,
        aters: string,
    }
}

export class ImageMsg extends jspb.Message { 
    getPath(): string;
    setPath(value: string): ImageMsg;
    getReceiver(): string;
    setReceiver(value: string): ImageMsg;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): ImageMsg.AsObject;
    static toObject(includeInstance: boolean, msg: ImageMsg): ImageMsg.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: ImageMsg, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): ImageMsg;
    static deserializeBinaryFromReader(message: ImageMsg, reader: jspb.BinaryReader): ImageMsg;
}

export namespace ImageMsg {
    export type AsObject = {
        path: string,
        receiver: string,
    }
}

export class MsgTypes extends jspb.Message { 

    getTypesMap(): jspb.Map<number, string>;
    clearTypesMap(): void;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): MsgTypes.AsObject;
    static toObject(includeInstance: boolean, msg: MsgTypes): MsgTypes.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: MsgTypes, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): MsgTypes;
    static deserializeBinaryFromReader(message: MsgTypes, reader: jspb.BinaryReader): MsgTypes;
}

export namespace MsgTypes {
    export type AsObject = {

        typesMap: Array<[number, string]>,
    }
}

export class Contact extends jspb.Message { 
    getWxid(): string;
    setWxid(value: string): Contact;
    getCode(): string;
    setCode(value: string): Contact;
    getName(): string;
    setName(value: string): Contact;
    getCountry(): string;
    setCountry(value: string): Contact;
    getProvince(): string;
    setProvince(value: string): Contact;
    getCity(): string;
    setCity(value: string): Contact;
    getGender(): number;
    setGender(value: number): Contact;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): Contact.AsObject;
    static toObject(includeInstance: boolean, msg: Contact): Contact.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: Contact, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): Contact;
    static deserializeBinaryFromReader(message: Contact, reader: jspb.BinaryReader): Contact;
}

export namespace Contact {
    export type AsObject = {
        wxid: string,
        code: string,
        name: string,
        country: string,
        province: string,
        city: string,
        gender: number,
    }
}

export class Contacts extends jspb.Message { 
    clearContactsList(): void;
    getContactsList(): Array<Contact>;
    setContactsList(value: Array<Contact>): Contacts;
    addContacts(value?: Contact, index?: number): Contact;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): Contacts.AsObject;
    static toObject(includeInstance: boolean, msg: Contacts): Contacts.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: Contacts, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): Contacts;
    static deserializeBinaryFromReader(message: Contacts, reader: jspb.BinaryReader): Contacts;
}

export namespace Contacts {
    export type AsObject = {
        contactsList: Array<Contact.AsObject>,
    }
}

export class DbNames extends jspb.Message { 
    clearNamesList(): void;
    getNamesList(): Array<string>;
    setNamesList(value: Array<string>): DbNames;
    addNames(value: string, index?: number): string;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): DbNames.AsObject;
    static toObject(includeInstance: boolean, msg: DbNames): DbNames.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: DbNames, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): DbNames;
    static deserializeBinaryFromReader(message: DbNames, reader: jspb.BinaryReader): DbNames;
}

export namespace DbNames {
    export type AsObject = {
        namesList: Array<string>,
    }
}

export class String extends jspb.Message { 
    getStr(): string;
    setStr(value: string): String;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): String.AsObject;
    static toObject(includeInstance: boolean, msg: String): String.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: String, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): String;
    static deserializeBinaryFromReader(message: String, reader: jspb.BinaryReader): String;
}

export namespace String {
    export type AsObject = {
        str: string,
    }
}

export class DbTable extends jspb.Message { 
    getName(): string;
    setName(value: string): DbTable;
    getSql(): string;
    setSql(value: string): DbTable;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): DbTable.AsObject;
    static toObject(includeInstance: boolean, msg: DbTable): DbTable.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: DbTable, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): DbTable;
    static deserializeBinaryFromReader(message: DbTable, reader: jspb.BinaryReader): DbTable;
}

export namespace DbTable {
    export type AsObject = {
        name: string,
        sql: string,
    }
}

export class DbTables extends jspb.Message { 
    clearTablesList(): void;
    getTablesList(): Array<DbTable>;
    setTablesList(value: Array<DbTable>): DbTables;
    addTables(value?: DbTable, index?: number): DbTable;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): DbTables.AsObject;
    static toObject(includeInstance: boolean, msg: DbTables): DbTables.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: DbTables, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): DbTables;
    static deserializeBinaryFromReader(message: DbTables, reader: jspb.BinaryReader): DbTables;
}

export namespace DbTables {
    export type AsObject = {
        tablesList: Array<DbTable.AsObject>,
    }
}

export class DbQuery extends jspb.Message { 
    getDb(): string;
    setDb(value: string): DbQuery;
    getSql(): string;
    setSql(value: string): DbQuery;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): DbQuery.AsObject;
    static toObject(includeInstance: boolean, msg: DbQuery): DbQuery.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: DbQuery, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): DbQuery;
    static deserializeBinaryFromReader(message: DbQuery, reader: jspb.BinaryReader): DbQuery;
}

export namespace DbQuery {
    export type AsObject = {
        db: string,
        sql: string,
    }
}

export class DbField extends jspb.Message { 
    getType(): number;
    setType(value: number): DbField;
    getColumn(): string;
    setColumn(value: string): DbField;
    getContent(): Uint8Array | string;
    getContent_asU8(): Uint8Array;
    getContent_asB64(): string;
    setContent(value: Uint8Array | string): DbField;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): DbField.AsObject;
    static toObject(includeInstance: boolean, msg: DbField): DbField.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: DbField, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): DbField;
    static deserializeBinaryFromReader(message: DbField, reader: jspb.BinaryReader): DbField;
}

export namespace DbField {
    export type AsObject = {
        type: number,
        column: string,
        content: Uint8Array | string,
    }
}

export class DbRow extends jspb.Message { 
    clearFieldsList(): void;
    getFieldsList(): Array<DbField>;
    setFieldsList(value: Array<DbField>): DbRow;
    addFields(value?: DbField, index?: number): DbField;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): DbRow.AsObject;
    static toObject(includeInstance: boolean, msg: DbRow): DbRow.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: DbRow, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): DbRow;
    static deserializeBinaryFromReader(message: DbRow, reader: jspb.BinaryReader): DbRow;
}

export namespace DbRow {
    export type AsObject = {
        fieldsList: Array<DbField.AsObject>,
    }
}

export class DbRows extends jspb.Message { 
    clearRowsList(): void;
    getRowsList(): Array<DbRow>;
    setRowsList(value: Array<DbRow>): DbRows;
    addRows(value?: DbRow, index?: number): DbRow;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): DbRows.AsObject;
    static toObject(includeInstance: boolean, msg: DbRows): DbRows.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: DbRows, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): DbRows;
    static deserializeBinaryFromReader(message: DbRows, reader: jspb.BinaryReader): DbRows;
}

export namespace DbRows {
    export type AsObject = {
        rowsList: Array<DbRow.AsObject>,
    }
}

export class Verification extends jspb.Message { 
    getV3(): string;
    setV3(value: string): Verification;
    getV4(): string;
    setV4(value: string): Verification;

    serializeBinary(): Uint8Array;
    toObject(includeInstance?: boolean): Verification.AsObject;
    static toObject(includeInstance: boolean, msg: Verification): Verification.AsObject;
    static extensions: {[key: number]: jspb.ExtensionFieldInfo<jspb.Message>};
    static extensionsBinary: {[key: number]: jspb.ExtensionFieldBinaryInfo<jspb.Message>};
    static serializeBinaryToWriter(message: Verification, writer: jspb.BinaryWriter): void;
    static deserializeBinary(bytes: Uint8Array): Verification;
    static deserializeBinaryFromReader(message: Verification, reader: jspb.BinaryReader): Verification;
}

export namespace Verification {
    export type AsObject = {
        v3: string,
        v4: string,
    }
}
