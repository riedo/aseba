#pragma once
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <aseba/common/msg/msg.h>
#include <boost/endian/arithmetic.hpp>
#include <iostream>

namespace mobsya {

template <class AsyncReadStream, class Handler>
class read_aseba_message_op;

using read_aseba_message_op_cb_t = void(boost::system::error_code, std::shared_ptr<Aseba::Message>);

template <class AsyncReadStream, class CompletionToken>
BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken, read_aseba_message_op_cb_t)
async_read_aseba_message(AsyncReadStream& stream, CompletionToken&& token) {
    static_assert(boost::beast::is_async_read_stream<AsyncReadStream>::value, "AsyncReadStream requirements not met");

    boost::asio::async_completion<CompletionToken, read_aseba_message_op_cb_t> init{token};
    read_aseba_message_op<AsyncReadStream, BOOST_ASIO_HANDLER_TYPE(CompletionToken, read_aseba_message_op_cb_t)>{
        stream, std::forward<CompletionToken>(init.completion_handler)}();

    return init.result.get();
}


template <class AsyncReadStream, class Handler>
class read_aseba_message_op {
    struct state {
        AsyncReadStream& stream;
        uint16_t size = 0;
        char sizeBuffer[2];
        Aseba::Message::SerializationBuffer dataBuffer;

        explicit state(Handler const& handler, AsyncReadStream& stream) : stream(stream) {}
    };
    boost::beast::handler_ptr<state, Handler> m_p;

public:
    read_aseba_message_op(read_aseba_message_op&&) = default;
    read_aseba_message_op(read_aseba_message_op const&) = default;

    template <class DeducedHandler, class... Args>
    read_aseba_message_op(AsyncReadStream& stream, DeducedHandler&& handler)
        : m_p(std::forward<DeducedHandler>(handler), stream) {}

    using allocator_type = boost::asio::associated_allocator_t<Handler>;

    allocator_type get_allocator() const noexcept {
        return boost::asio::get_associated_allocator(m_p.handler());
    }

    using executor_type =
        boost::asio::associated_executor_t<Handler, decltype(std::declval<AsyncReadStream&>().get_executor())>;

    executor_type get_executor() const noexcept {
        return boost::asio::get_associated_executor(m_p.handler(), m_p->stream.get_executor());
    }

    void operator()() {
        auto& state = *m_p;
        return boost::asio::async_read(state.stream, boost::asio::buffer(state.sizeBuffer),
                                       boost::asio::transfer_exactly(2), std::move(*this));
    }

    void operator()(boost::system::error_code ec, std::size_t bytes_transferred) {
        auto& state = *m_p;
        if(!ec && bytes_transferred == 0)
            return state.size == 0 ?
                    boost::asio::async_read(state.stream, boost::asio::buffer(state.sizeBuffer),
                                           boost::asio::transfer_exactly(2), std::move(*this))
                        : boost::asio::async_read(state.stream, boost::asio::buffer(state.dataBuffer.rawData),
                                                  boost::asio::transfer_exactly(state.size + 4), std::move(*this));
        if(!ec && state.size == 0) {
            assert(bytes_transferred == 2);
            state.size = boost::endian::little_to_native(reinterpret_cast<uint16_t&>(state.sizeBuffer));
            state.dataBuffer.rawData.resize(state.size + 4);
            return boost::asio::async_read(state.stream, boost::asio::buffer(state.dataBuffer.rawData),
                                           boost::asio::transfer_exactly(state.size + 4), std::move(*this));
        }
        if(!ec && bytes_transferred == state.size + 4) {
            Aseba::Message::SerializationBuffer& b = state.dataBuffer;
            auto source = b.get<uint16_t>();
            auto type = b.get<uint16_t>();
            auto msg = std::shared_ptr<Aseba::Message>(Aseba::Message::create(source, type, state.dataBuffer));
            m_p.invoke(ec, std::move(msg));
            return;
        }
        m_p.invoke(ec, std::shared_ptr<Aseba::Message>{});
    }
};
}  // namespace mobsya
