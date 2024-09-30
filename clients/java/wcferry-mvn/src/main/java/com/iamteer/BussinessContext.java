package com.iamteer;

import org.springframework.core.annotation.Order;
import org.springframework.stereotype.Component;

import lombok.Data;
import lombok.extern.slf4j.Slf4j;

@Slf4j
@Component
@Data
@Order(100)
public class BussinessContext {

    private Client client;

}
