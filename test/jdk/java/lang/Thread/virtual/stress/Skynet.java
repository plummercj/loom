/*
 * Copyright (c) 2022, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/**
 * @test
 * @requires vm.debug != true
 * @compile --enable-preview -source ${jdk.version} Skynet.java
 * @run main/othervm --enable-preview Skynet
 * @summary Skynet benchmark
 */

 /**
 * @test
 * @requires vm.debug == true
 * @compile --enable-preview -source ${jdk.version} Skynet.java
 * @run main/othervm/timeout=300 --enable-preview Skynet
 */

/**
 * @test
 * @requires vm.debug == true
 * @requires vm.gc.Z
 * @compile --enable-preview -source ${jdk.version} Skynet.java
 * @run main/othervm/timeout=300 --enable-preview -XX:+UnlockDiagnosticVMOptions -XX:+ZVerifyViews -XX:ZCollectionInterval=0.01 Skynet
 */

import java.util.concurrent.BlockingQueue;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadFactory;

public class Skynet {
    public static final int ITERATIONS = 10;
    public static void main(String[] args) {
        for (int i=0; i<ITERATIONS; i++) {
            skynet(1_000_000, 499999500000L);
        }
    }

    static void skynet(int num, long expected) {
        long start = System.currentTimeMillis();
        var chan = new Channel<Long>();

        Thread.startVirtualThread(() -> skynet(chan, 0, num, 10));

        long sum = chan.receive();
        long end = System.currentTimeMillis();
        System.out.format("Result: %d in %s ms%n", sum, (end-start));
        if (sum != expected)
            throw new AssertionError("unexpected result!");
    }

    static void skynet(Channel<Long> result, int num, int size, int div) {
        if (size == 1) {
            result.send((long)num);
        } else {
            var chan = new Channel<Long>();
            for (int i = 0; i < div; i++) {
                int subNum = num + i * (size / div);
                Thread.startVirtualThread(() -> skynet(chan, subNum, size / div, div));
            }
            long sum = 0;
            for (int i = 0; i < div; i++) {
                sum += chan.receive();
            }
            result.send(sum);
        }
    }

    static class Channel<T> {
        private final BlockingQueue<T> q;

        Channel() {
            q = new SynchronousQueue<>();
        }

        void send(T e) {
            boolean interrupted = false;
            while (true) {
                try {
                    q.put(e);
                    break;
                } catch (InterruptedException x) {
                    interrupted = true;
                }
            }
            if (interrupted)
                Thread.currentThread().interrupt();
        }

        T receive() {
            boolean interrupted = false;
            T e;
            while (true) {
                try {
                    e = q.take();
                    break;
                } catch (InterruptedException x) {
                    interrupted = true;
                }
            }
            if (interrupted)
                Thread.currentThread().interrupt();
            return e;
        }
    }
}
